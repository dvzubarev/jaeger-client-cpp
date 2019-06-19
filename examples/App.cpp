#include <iostream>

#include <yaml-cpp/yaml.h>

#include <jaegertracing/Tracer.h>



namespace jaeger_ceph {


void setUpTracer(const char* serviceToTrace)
{
    const char* configFileAddress = "/home/d/config.yml";
    auto configYAML = YAML::LoadFile(configFileAddress);
    auto config = jaegertracing::Config::parse(configYAML);
    auto tracer = jaegertracing::Tracer::make(
       serviceToTrace, config, jaegertracing::logging::consoleLogger());
    opentracing::Tracer::InitGlobal(
        std::static_pointer_cast<opentracing::Tracer>(tracer));
}
const std::unique_ptr<opentracing::Span> tracedSubroutine(
    const std::unique_ptr<opentracing::Span>& parentSpan, 
    const char* subRoutineContext)
{
    auto span = opentracing::Tracer::Global()->StartSpan(
       subRoutineContext, { opentracing::ChildOf(&parentSpan->context()) });
    return span;
}

const std::unique_ptr<opentracing::Span> tracedFunction(const char* funcContext)
// void tracedFunction(const char* funcContext)
{
    auto span = opentracing::Tracer::Global()->StartSpan(funcContext);
    return span;
}

 std::string inject(const std::unique_ptr<opentracing::Span>& span, const char *name)
  {
    std::stringstream ss;
    if (!span) {
	auto span = opentracing::Tracer::Global()->StartSpan(name);
    }
    auto err = opentracing::Tracer::Global()->Inject(span->context(), ss);
    assert(err);
    return ss.str();
  }

  void extract(const std::unique_ptr<opentracing::Span>& span, const char *name, std::string t_meta)
  {
    std::stringstream ss(t_meta);
    //if(!tracer){
    //}
    //setUpTracer("Extract-service");
    auto span_context_maybe = opentracing::Tracer::Global()->Extract(ss);
    assert(span_context_maybe);

    //Propogation span
    auto _span = opentracing::Tracer::Global()->StartSpan("propagationSpan",
	{ChildOf(span_context_maybe->get())});

    //span1 = std::move(_span);
    auto span1 = std::move(_span);

  }
}

int main()
{

    jaeger_ceph::setUpTracer("f");
    const std::unique_ptr<opentracing::Span>& span = jaeger_ceph::tracedFunction("funcCtx");
    jaeger_ceph::tracedSubroutine(span,"subroutineCtx");
//  assert(parentSpan);
    
//    std::string inject_test = jaeger_ceph::inject(parentSpan,"inject_placeholder_const_char");
//    jaeger_ceph::extract(parentSpan, "hello-extract", "metadata_placeholder"); 

    opentracing::Tracer::Global()->Close();
    return 0;
}

