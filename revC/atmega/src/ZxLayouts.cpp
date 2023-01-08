#include "ZxLayout.h"
#include "ZxKeyboard.h"

extern const ZxLayoutKeyRef NormalLayout[256];
extern const ZxLayoutKeyRef LeftShiftLayout[256];
extern const ZxLayoutKeyRef AlasmNormalLayout[256];
extern const ZxLayoutKeyRef AlasmLeftShiftLayout[256];
extern const ZxLayoutKeyRef AlasmRusNormalLayout[256];
extern const ZxLayoutKeyRef AlasmRusLeftShiftLayout[256];

const ZxLayoutReference LayoutReferences[4] STORAGE = {
        {
                // Base keyboard layout
                .BaseLayout = &NormalLayout[0],
                .LeftShiftLayout = &LeftShiftLayout[0],
                .RusLayout = &NormalLayout[0],
                .LeftShiftRusLayout = &NormalLayout[0],
                .FallbackLayout = &NormalLayout[0]
        },
        {
                // Alasm keys layout
                .BaseLayout = &AlasmNormalLayout[0],
                .LeftShiftLayout = &AlasmLeftShiftLayout[0],
                .RusLayout = &AlasmRusNormalLayout[0],
                .LeftShiftRusLayout = &AlasmRusLeftShiftLayout[0],
                .FallbackLayout = &NormalLayout[0]
        },
        {
                // Base keyboard layout
                .BaseLayout = &NormalLayout[0],
                .LeftShiftLayout = &LeftShiftLayout[0],
                .RusLayout = &NormalLayout[0],
                .LeftShiftRusLayout = &NormalLayout[0],
                .FallbackLayout = &NormalLayout[0]
        },
        {
                // Base keyboard layout
                .BaseLayout = &NormalLayout[0],
                .LeftShiftLayout = &LeftShiftLayout[0],
                .RusLayout = &NormalLayout[0],
                .LeftShiftRusLayout = &NormalLayout[0],
                .FallbackLayout = &NormalLayout[0]
        }
};