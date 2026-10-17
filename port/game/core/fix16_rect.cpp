#include "core/fix16_rect.hpp"
#include "SpriteGrid_400.hpp"
#include "map_0x370.hpp"
#include "sprite.hpp"

DEFINE_GLOBAL_INIT(Fix16, kSmallWidthEpslion, Fix16(0xCCC, 0), 0x703450);
DEFINE_GLOBAL_INIT(Fix16, kCollisionPrismHalfHeight, Fix16(0x800, 0), 0x6771E4);

// 0x4E6280
void Fix16Rect::MakeRect(Fix16 x, Fix16 y, Fix16 w, Fix16 h)
{
    left_ = x;
    top_ = y;
    right_ = x + w;
    bottom_ = y + h;
}

// 0x59dd60
void Fix16Rect::DoSetCurrentRect()
{
    gSpriteGrid_left_6F5FD4 = left_.ToInt();
    gSpriteGrid_right_6F5B80 = right_.ToInt();
    gSpriteGrid_top_6F6108 = top_.ToInt();
    gSpriteGrid_bottom_6F5F38 = bottom_.ToInt();
}

// 0x4BA6C0 9.6f
// 0x59ddf0
bool Fix16Rect::IntersectsSpriteRenderingRect(Sprite* sprite)
{
    Fix16Point* pCorners = sprite->field_C_pCollisionShape->field_C_corners;

    return (IsPointInRect(&pCorners[0]) || IsPointInRect(&pCorners[1]) || IsPointInRect(&pCorners[2]) || IsPointInRect(&pCorners[3]));
}

// 0x59de80
char_type Fix16Rect::CanRectEnterMovementRegion()
{
    DoSetCurrentRect();
    s32 x = ((this->left_ + this->right_) / 2).ToInt();
    s32 y = ((this->top_ + this->bottom_) / 2).ToInt();
    s32 z = ((this->low_z_ + this->high_z_) / 2).ToInt();
    return gMap_0x370_6F6268->CanSpriteEnterMovementRegion_4E4460(x, y, z, nullptr, 1024);
}

// 0x59dee0
void Fix16Rect::ExpandToIncludePoint(Fix16 x, Fix16 y)
{
    if (x < left_)
    {
        left_ = x;
    }
    else
    {
        if (x > right_)
        {
            right_ = x;
        }
    }

    if (y < top_)
    {
        top_ = y;
    }
    else
    {
        if (y > bottom_)
        {
            bottom_ = y;
        }
    }
}

// 0x59df20
bool Fix16Rect::RectOverlapsZone(u8 zone_type)
{
    u8 zone_y = top_.ToUInt8();
    u8 zone_x;

    while (zone_y <= bottom_.ToUInt8())
    {
        zone_x = left_.ToUInt8();
        while (zone_x <= right_.ToUInt8())
        {
            if (gMap_0x370_6F6268->zone_by_pos_and_type_4DF4D0(zone_x, zone_y, zone_type))
            {
                return true;
            }
            ++zone_x;
        }
        ++zone_y;
    }
    return false;
}

// 0x59dfb0
bool Fix16Rect::EveryTileMatchesArrowType(s32 arrow_type)
{
    u8 y_pos = top_.ToUInt8();
    u8 x_pos;

    bool bMatches = true;

    while (y_pos <= bottom_.ToUInt8())
    {
        x_pos = left_.ToUInt8();
        while (x_pos <= right_.ToUInt8())
        {
            if (!bMatches)
            {
                return false;
            }

            gmp_block_info* block_4DFE10 = gMap_0x370_6F6268->get_block_4DFE10(x_pos, y_pos, low_z_.ToInt());
            if (block_4DFE10)
            {

                switch (arrow_type)
                {
                    case 1:
                        bMatches = (block_4DFE10->field_A_arrows & block_arrow::green_mask) == block_arrow::green_up_4; //  green arrow up
                        break;
                    case 2:
                        bMatches =
                            (block_4DFE10->field_A_arrows & block_arrow::green_mask) == block_arrow::green_down_8; //  green arrow down
                        break;
                    case 3:
                        bMatches =
                            (block_4DFE10->field_A_arrows & block_arrow::green_mask) == block_arrow::green_right_2; //  green arrow right
                        break;
                    case 4:
                        bMatches =
                            (block_4DFE10->field_A_arrows & block_arrow::green_mask) == block_arrow::green_left_1; //  green arrow left
                        break;
                    default:
                        break;
                }
            }
            else
            {
                bMatches = false;
            }
            ++x_pos;
        }
        ++y_pos;
    }
    return true;
}

// 0x5A5E30
void Fix16Rect::SetRectOutOfLine(Fix16 left, Fix16 right, Fix16 top, Fix16 bottom)
{
    this->left_ = left;
    this->right_ = right;
    this->top_ = top;
    this->bottom_ = bottom;
}