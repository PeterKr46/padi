//
// Created by Peter on 28/04/2022.
//

#pragma once

#include <functional>

namespace padi {

    class Stage;

    // Returns true if to be repeated for next frame
    typedef std::function<bool(padi::Stage *)> FrameListener;

} // padi

