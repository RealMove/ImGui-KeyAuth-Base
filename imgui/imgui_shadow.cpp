#include "imgui_shadow.h"

ImVec2 operator+(ImVec2 p1, ImVec2 p2) { return ImVec2(p1.x + p2.x, p1.y + p2.y); }
ImVec2 operator-(ImVec2 p1, ImVec2 p2) { return ImVec2(p1.x - p2.x, p1.y - p2.y); }
ImVec2 operator/(ImVec2 p1, ImVec2 p2) { return ImVec2(p1.x / p2.x, p1.y / p2.y); }
ImVec2 operator*(ImVec2 p1, int value) { return ImVec2(p1.x * value, p1.y * value); }
ImVec2 operator*(ImVec2 p1, float value) { return ImVec2(p1.x * value, p1.y * value); }
ImVec4 operator+(float val, ImVec4 p2) { return ImVec4(val + p2.x, val + p2.y, val + p2.z, val + p2.w); }
ImVec4 operator*(float val, ImVec4 p2) { return ImVec4(val * p2.x, val * p2.y, val * p2.z, val * p2.w); }
ImVec4 operator*(ImVec4 p2, float val) { return ImVec4(val * p2.x, val * p2.y, val * p2.z, val * p2.w); }
ImVec4 operator-(ImVec4 p1, ImVec4 p2) { return ImVec4(p1.x - p2.x, p1.y - p2.y, p1.z - p2.z, p1.w - p2.w); }
ImVec4 operator*(ImVec4 p1, ImVec4 p2) { return ImVec4(p1.x * p2.x, p1.y * p2.y, p1.z * p2.z, p1.w * p2.w); }
ImVec4 operator/(ImVec4 p1, ImVec4 p2) { return ImVec4(p1.x / p2.x, p1.y / p2.y, p1.z / p2.z, p1.w / p2.w); }

ImVec4 boxGaussianIntegral(ImVec4 x)
{
    const ImVec4 s = ImVec4(x.x > 0 ? 1.0f : -1.0f, x.y > 0 ? 1.0f : -1.0f, x.z > 0 ? 1.0f : -1.0f, x.w > 0 ? 1.0f : -1.0f);
    const ImVec4 a = ImVec4(fabsf(x.x), fabsf(x.y), fabsf(x.z), fabsf(x.w));
    const ImVec4 res = 1.0f + (0.278393f + (0.230389f + 0.078108f * (a * a)) * a) * a;
    const ImVec4 resSquared = res * res;
    return s - s / (resSquared * resSquared);
}

ImVec4 boxLinearInterpolation(ImVec4 x)
{
    const float maxClamp = 1.0f;
    const float minClamp = -1.0f;
    return ImVec4(x.x > maxClamp ? maxClamp : x.x < minClamp ? minClamp : x.x,
        x.y > maxClamp ? maxClamp : x.y < minClamp ? minClamp : x.y,
        x.z > maxClamp ? maxClamp : x.z < minClamp ? minClamp : x.z,
        x.w > maxClamp ? maxClamp : x.w < minClamp ? minClamp : x.w);
}

float boxShadow(ImVec2 lower, ImVec2 upper, ImVec2 point, float sigma, bool linearInterpolation)
{
    const ImVec2 pointLower = point - lower;
    const ImVec2 pointUpper = point - upper;
    const ImVec4 query = ImVec4(pointLower.x, pointLower.y, pointUpper.x, pointUpper.y);
    const ImVec4 pointToSample = query * (sqrtf(0.5f) / sigma);
    const ImVec4 integral = linearInterpolation ? 0.5f + 0.5f * boxLinearInterpolation(pointToSample) : 0.5f + 0.5f * boxGaussianIntegral(pointToSample);
    return (integral.z - integral.x) * (integral.w - integral.y);
}

void ImGui::AddShadow(ImVec2 rectPos, ImVec2 rectSize, float sigma, int rings, int spacingBetweenRings, int samplesPerCornerSide, int spacingBetweenSamples, ImColor shadowColor, ImVec2 shadowOffset)
{
    const int    samplesSpan = samplesPerCornerSide * spacingBetweenSamples;
    const int    halfWidth = static_cast<int>(rectSize.x / 2);
    const int    numSamplesInHalfWidth = (halfWidth / spacingBetweenSamples) == 0 ? 1 : halfWidth / spacingBetweenSamples;
    const int    numSamplesWidth = samplesSpan > halfWidth ? numSamplesInHalfWidth : samplesPerCornerSide;
    const int    halfHeight = static_cast<int>(rectSize.y / 2);
    const int    numSamplesInHalfHeight = (halfHeight / spacingBetweenSamples) == 0 ? 1 : halfHeight / spacingBetweenSamples;
    const int    numSamplesHeight = samplesSpan > halfHeight ? numSamplesInHalfHeight : samplesPerCornerSide;
    const int    numVerticesInARing = numSamplesWidth * 4 + numSamplesHeight * 4 + 4;
    const ImVec2 whiteTexelUV = ImGui::GetIO().Fonts->TexUvWhitePixel;
    const ImVec2 origin = ImGui::GetCursorScreenPos();
    const ImVec2 rectangleTopLeft = origin + rectPos;
    const ImVec2 rectangleBottomRight = rectangleTopLeft + rectSize;
    const ImVec2 rectangleTopRight = rectangleTopLeft + ImVec2(rectSize.x, 0);
    const ImVec2 rectangleBottomLeft = rectangleTopLeft + ImVec2(0, rectSize.y);

    int totalVertices = numVerticesInARing * rings;
    int totalIndices = 6 * (numVerticesInARing) * (rings - 1);

    ImDrawList* drawList = ImGui::GetBackgroundDrawList();
    drawList->PrimReserve(totalIndices, totalVertices);
    const ImDrawVert* shadowVertices = drawList->_VtxWritePtr;
    ImDrawVert* vertexPointer = drawList->_VtxWritePtr;

    for (int r = 0; r < rings; ++r)
    {
        const float  adaptiveScale = (r / 2.5f) + 1;
        const ImVec2 ringOffset = ImVec2(adaptiveScale * r * spacingBetweenRings, adaptiveScale * r * spacingBetweenRings);
        for (int j = 0; j < 4; ++j)
        {
            ImVec2      corner;
            ImVec2      direction[2];
            const float spacingBetweenSamplesOnARing = static_cast<float>(spacingBetweenSamples);
            switch (j)
            {
            case 0:
                corner = rectangleTopLeft + ImVec2(-ringOffset.x, -ringOffset.y);
                direction[0] = ImVec2(1, 0) * spacingBetweenSamplesOnARing;
                direction[1] = ImVec2(0, 1) * spacingBetweenSamplesOnARing;
                for (int i = 0; i < numSamplesWidth; ++i)
                {
                    const ImVec2 point = corner + direction[0] * (numSamplesWidth - i);
                    shadowColor.Value.w = boxShadow(rectangleTopLeft, rectangleBottomRight, point - shadowOffset, sigma, false);
                    vertexPointer->pos = point;
                    vertexPointer->uv = whiteTexelUV;
                    vertexPointer->col = shadowColor;
                    vertexPointer++;
                }

                shadowColor.Value.w = boxShadow(rectangleTopLeft, rectangleBottomRight, corner - shadowOffset, sigma, false);
                vertexPointer->pos = corner;
                vertexPointer->uv = whiteTexelUV;
                vertexPointer->col = shadowColor;
                vertexPointer++;

                for (int i = 0; i < numSamplesHeight; ++i)
                {
                    const ImVec2 point = corner + direction[1] * (i + 1);
                    shadowColor.Value.w = boxShadow(rectangleTopLeft, rectangleBottomRight, point - shadowOffset, sigma, false);
                    vertexPointer->pos = point;
                    vertexPointer->uv = whiteTexelUV;
                    vertexPointer->col = shadowColor;
                    vertexPointer++;
                }
                break;
            case 1:
                corner = rectangleBottomLeft + ImVec2(-ringOffset.x, +ringOffset.y);
                direction[0] = ImVec2(1, 0) * spacingBetweenSamplesOnARing;
                direction[1] = ImVec2(0, -1) * spacingBetweenSamplesOnARing;
                for (int i = 0; i < numSamplesHeight; ++i)
                {
                    const ImVec2 point = corner + direction[1] * (numSamplesHeight - i);
                    shadowColor.Value.w = boxShadow(rectangleTopLeft, rectangleBottomRight, point - shadowOffset, sigma, false);
                    vertexPointer->pos = point;
                    vertexPointer->uv = whiteTexelUV;
                    vertexPointer->col = shadowColor;
                    vertexPointer++;
                }

                shadowColor.Value.w = boxShadow(rectangleTopLeft, rectangleBottomRight, corner - shadowOffset, sigma, false);
                vertexPointer->pos = corner;
                vertexPointer->uv = whiteTexelUV;
                vertexPointer->col = shadowColor;
                vertexPointer++;

                for (int i = 0; i < numSamplesWidth; ++i)
                {
                    const ImVec2 point = corner + direction[0] * (i + 1);
                    shadowColor.Value.w = boxShadow(rectangleTopLeft, rectangleBottomRight, point - shadowOffset, sigma, false);
                    vertexPointer->pos = point;
                    vertexPointer->uv = whiteTexelUV;
                    vertexPointer->col = shadowColor;
                    vertexPointer++;
                }
                break;
            case 2:
                corner = rectangleBottomRight + ImVec2(+ringOffset.x, +ringOffset.y);
                direction[0] = ImVec2(-1, 0) * spacingBetweenSamplesOnARing;
                direction[1] = ImVec2(0, -1) * spacingBetweenSamplesOnARing;
                for (int i = 0; i < numSamplesWidth; ++i)
                {
                    const ImVec2 point = corner + direction[0] * (numSamplesWidth - i);
                    shadowColor.Value.w = boxShadow(rectangleTopLeft, rectangleBottomRight, point - shadowOffset, sigma, false);
                    vertexPointer->pos = point;
                    vertexPointer->uv = whiteTexelUV;
                    vertexPointer->col = shadowColor;
                    vertexPointer++;
                }

                shadowColor.Value.w = boxShadow(rectangleTopLeft, rectangleBottomRight, corner - shadowOffset, sigma, false);
                vertexPointer->pos = corner;
                vertexPointer->uv = whiteTexelUV;
                vertexPointer->col = shadowColor;
                vertexPointer++;

                for (int i = 0; i < numSamplesHeight; ++i)
                {
                    const ImVec2 point = corner + direction[1] * (i + 1);
                    shadowColor.Value.w = boxShadow(rectangleTopLeft, rectangleBottomRight, point - shadowOffset, sigma, false);
                    vertexPointer->pos = point;
                    vertexPointer->uv = whiteTexelUV;
                    vertexPointer->col = shadowColor;
                    vertexPointer++;
                }
                break;
            case 3:
                corner = rectangleTopRight + ImVec2(+ringOffset.x, -ringOffset.y);
                direction[0] = ImVec2(-1, 0) * spacingBetweenSamplesOnARing;
                direction[1] = ImVec2(0, 1) * spacingBetweenSamplesOnARing;
                for (int i = 0; i < numSamplesHeight; ++i)
                {
                    const ImVec2 point = corner + direction[1] * (numSamplesHeight - i);
                    shadowColor.Value.w = boxShadow(rectangleTopLeft, rectangleBottomRight, point - shadowOffset, sigma, false);
                    vertexPointer->pos = point;
                    vertexPointer->uv = whiteTexelUV;
                    vertexPointer->col = shadowColor;
                    vertexPointer++;
                }

                shadowColor.Value.w = boxShadow(rectangleTopLeft, rectangleBottomRight, corner - shadowOffset, sigma, false);
                vertexPointer->pos = corner;
                vertexPointer->uv = whiteTexelUV;
                vertexPointer->col = shadowColor;
                vertexPointer++;

                for (int i = 0; i < numSamplesWidth; ++i)
                {
                    const ImVec2 point = corner + direction[0] * (i + 1);
                    shadowColor.Value.w = boxShadow(rectangleTopLeft, rectangleBottomRight, point - shadowOffset, sigma, false);
                    vertexPointer->pos = point;
                    vertexPointer->uv = whiteTexelUV;
                    vertexPointer->col = shadowColor;
                    vertexPointer++;
                }
                break;
            }
        }
    }

    ImDrawIdx idx = (ImDrawIdx)drawList->_VtxCurrentIdx;

    for (int r = 0; r < rings - 1; ++r)
    {
        const ImDrawIdx startOfRingIndex = idx;
        for (int i = 0; i < numVerticesInARing - 1; ++i)
        {
            drawList->_IdxWritePtr[0] = idx + 0;
            drawList->_IdxWritePtr[1] = idx + 1;
            drawList->_IdxWritePtr[2] = idx + numVerticesInARing;
            drawList->_IdxWritePtr[3] = idx + 1;
            drawList->_IdxWritePtr[4] = idx + numVerticesInARing + 1;
            drawList->_IdxWritePtr[5] = idx + numVerticesInARing;

            idx += 1;
            drawList->_IdxWritePtr += 6;
        }

        drawList->_IdxWritePtr[0] = idx + 0;
        drawList->_IdxWritePtr[1] = startOfRingIndex + 0;
        drawList->_IdxWritePtr[2] = startOfRingIndex + numVerticesInARing;
        drawList->_IdxWritePtr[3] = idx + 0;
        drawList->_IdxWritePtr[4] = startOfRingIndex + numVerticesInARing;
        drawList->_IdxWritePtr[5] = idx + numVerticesInARing;

        drawList->_IdxWritePtr += 6;
        idx += 1;
    }

    drawList->_VtxCurrentIdx += totalVertices;

}
