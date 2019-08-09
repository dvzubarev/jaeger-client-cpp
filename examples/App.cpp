#include <boost/type_index.hpp>
#include <iostream>

#include <yaml-cpp/yaml.h>

#include <jaegertracing/Tracer.h>

namespace {

std::shared_ptr<opentracing::Tracer> setUpTracer(const char* configFilePath)
{
    auto configYAML = YAML::LoadFile(configFilePath);
    auto config = jaegertracing::Config::parse(configYAML);
    std::cout << boost::typeindex::type_id<decltype(config)>().pretty_name()
              << std::endl;
    std::cout << config.disabled();
    // std::cout << config.sampler();
    auto tracer = jaegertracing::Tracer::make(
        "example-service", config, jaegertracing::logging::consoleLogger());
    opentracing::Tracer::InitGlobal(
        std::static_pointer_cast<opentracing::Tracer>(tracer));
    std::cout << boost::typeindex::type_id<decltype(tracer)>().pretty_name()
              << std::endl;
    return tracer;
}

void tracedSubroutine(const std::unique_ptr<opentracing::Span>& parentSpan)
{
    auto span = opentracing::Tracer::Global()->StartSpan(
        "tracedSubroutine", { opentracing::ChildOf(&parentSpan->context()) });
}

const std::unique_ptr<opentracing::Span> tracedFunction()
{
    auto span = opentracing::Tracer::Global()->StartSpan("tracedFunction");
    tracedSubroutine(span);
    span->Finish();
    return span;
}

static std::string inject(const char* name)
{
    std::stringstream ss;
    auto jspan = opentracing::Tracer::Global()->StartSpan("test");
    if (!jspan) {
        auto span = opentracing::Tracer::Global()->StartSpan(name);
    }
    auto err = opentracing::Tracer::Global()->Inject(jspan->context(), ss);
    return ss.str();
}
static void extract(const std::unique_ptr<opentracing::Span> span,
                    const char* name,
                    std::string t_meta)
{
    std::stringstream ss(t_meta);
    //    if(!tracer){
    //    }
    // setUpTracer("Extract-service");
    auto span_context_maybe = opentracing::Tracer::Global()->Extract(ss);
    assert(span_context_maybe);

    // Propogation span
    auto _span = opentracing::Tracer::Global()->StartSpan(
        "propagationSpan", { opentracing::ChildOf(span_context_maybe->get()) });

    auto span1 = std::move(_span);
}  // anonymous namespace
}  // namespace

int main(int argc, char* argv[])
{
    if (argc < 2) {
        std::cerr << "usage: " << argv[0] << " <config-yaml-path>\n";
        return 1;
    }
    auto tracer = setUpTracer(argv[1]);

    auto kspan = opentracing::Tracer::Global()->StartSpan("test");
    std::string t_meta = inject("inject");
    std::cout << t_meta;
    // Not stricly necessary to close tracer, but might flush any buffered
    // spans. See more details in opentracing::Tracer::Close() documentation.
    opentracing::Tracer::Global()->Close();
    return 0;
}

