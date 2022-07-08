/* Copyright (c) 2022, Arm Limited and Contributors
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 the "License";
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#pragma once

#include <functional>

#include "components/events/event_bus.hpp"

namespace components
{
namespace events
{
class PipelineStage
{
  public:
	PipelineStage()          = default;
	virtual ~PipelineStage() = default;

	virtual const char *name() const        = 0;
	virtual void        emit(EventBus &bus) = 0;
};

template <typename Event>
class TypedPipelineStage : public PipelineStage
{
  public:
	TypedPipelineStage()
	{}

	virtual ~TypedPipelineStage() = default;

	virtual const char *name() const override
	{
		return typeid(Event).name();
	}

	virtual void emit(EventBus &bus) override
	{
		auto sender = bus.request_sender<Event>();
		sender->push(Event{});
	}
};

template <typename Event, typename Func>
class TypedPipelineStageWithFunc : public TypedPipelineStage<Event>
{
  public:
	TypedPipelineStageWithFunc(Func &&func = []() -> Event { return Event{}; }) :
	    m_func{std::move(func)}
	{}

	virtual ~TypedPipelineStageWithFunc() = default;

	virtual void emit(EventBus &bus) override
	{
		auto sender = bus.request_sender<Event>();
		sender->push(m_func());
	}

  private:
	Func m_func;
};

class Pipeline : public EventBus
{
  public:
	Pipeline()          = default;
	virtual ~Pipeline() = default;

	Pipeline &once(std::unique_ptr<PipelineStage> &&stage);

	Pipeline &then(std::unique_ptr<PipelineStage> &&stage);

	virtual void process() override;

  protected:
	bool running{false};

	std::vector<std::unique_ptr<PipelineStage>> m_once_stages;
	std::vector<std::unique_ptr<PipelineStage>> m_stages;
};
}        // namespace events
}        // namespace components