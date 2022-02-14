#include "pca.h"
#include "components/update.h"
#include "components/pca.h"
#include "private/pca.h"

namespace System
{
    void PCA::setup_reactive_systems(entt::registry& registry)
    {
        using namespace Private::PCA;
        registry.set<SourceDataset>();
        registry.set<DestinationDataset>();
        reset(registry.set<Component::SourcePCA>());
        reset(registry.set<Component::DestinationPCA>());
        registry.on_update<Component::Update>().connect<&regenerate_pca>();
    }
}
