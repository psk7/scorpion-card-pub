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
                .BaseLayout = &NormalLayout[0],
                .LeftShiftLayout = &LeftShiftLayout[0],
                .RusLayout = &NormalLayout[0],
                .LeftShiftRusLayout = &LeftShiftLayout[0],
                .FallbackLayout = &NormalLayout[0]
        },
        {
                .BaseLayout = &NormalLayout[0],
                .LeftShiftLayout = &NormalLayout[0],
                .RusLayout = &NormalLayout[0],
                .LeftShiftRusLayout = &NormalLayout[0],
                .FallbackLayout = &NormalLayout[0]
        },
        {
                .BaseLayout = &NormalLayout[0],
                .LeftShiftLayout = &NormalLayout[0],
                .RusLayout = &NormalLayout[0],
                .LeftShiftRusLayout = &NormalLayout[0],
                .FallbackLayout = &NormalLayout[0]
        },
        {
                .BaseLayout = &NormalLayout[0],
                .LeftShiftLayout = &NormalLayout[0],
                .RusLayout = &NormalLayout[0],
                .LeftShiftRusLayout = &NormalLayout[0],
                .FallbackLayout = &NormalLayout[0]
        }
};