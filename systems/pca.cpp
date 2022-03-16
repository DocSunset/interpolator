#include "pca.h"
#include "components/update.h"
#include "components/pca.h"
#include "private/pca.h"

namespace System
{
    void PCA::setup_reactive_systems(entt::registry& registry)
    {
        using namespace Private::PCA;
        registry.set<SourceDataset>().entity = registry.create();
        registry.set<DestinationDataset>().entity = registry.create();

        reset(registry.set<Component::SourcePCA>());
        registry.emplace<Component::SourcePCA>(registry.ctx<SourceDataset>().entity
                , registry.ctx<Component::SourcePCA>());

        reset(registry.set<Component::DestinationPCA>());
        registry.emplace<Component::DestinationPCA>(registry.ctx<DestinationDataset>().entity
                , registry.ctx<Component::DestinationPCA>());

        registry.on_update<Component::Update>().connect<&regenerate_pca>();
    }
}
