extern "C"
{
#include "raylib.h"
#include "raymath.h"
#define RAYGUI_IMPLEMENTATION
#include "raygui.h"
}
#if defined(PLATFORM_WEB)
#include <emscripten/emscripten.h>
#endif

#include "array.h"

#include <initializer_list>
#include <functional>
#include <vector>
#include <string>
#include <algorithm>
#include <unordered_map>

//--------------------------------------

// Process union operation on arrays of ranges.
// Assumes `out` is pre-allocated to be large enough 
// to store result. Returns the number of ranges 
// generated as output.
int ranges_union(
    slice1d<range> out,
    const slice1d<range> lhs,
    const slice1d<range> rhs)
{
    // Activation state of each list of ranges
    bool out_active = false;
    bool lhs_active = false;
    bool rhs_active = false;
    
    // Event index for each list of ranges
    int out_i = 0;
    int lhs_i = 0;
    int rhs_i = 0;
  
    // While both ranges have events to process
    while (lhs_i < lhs.size * 2 && rhs_i < rhs.size * 2)
    {
        // Are the next lhs, and rhs events active or inactive
        bool lhs_active_next = lhs_i % 2 == 0;
        bool rhs_active_next = rhs_i % 2 == 0;
        
        // Time of the next lhs, and rhs events
        int lhs_t = lhs_active_next ? lhs(lhs_i / 2).start : lhs(lhs_i / 2).stop;
        int rhs_t = rhs_active_next ? rhs(rhs_i / 2).start : rhs(rhs_i / 2).stop;
      
        // Event from lhs is coming first
        if (lhs_t < rhs_t)
        {
            // Activate output
            if (!out_active && lhs_active_next)
            {
                out_active = true;
                out(out_i).start = lhs_t;
            }
            // Deactivate output
            else if (out_active && !lhs_active_next && !rhs_active)
            {
                out_active = false;
                out(out_i).stop = lhs_t;
                out_i++;
            }
          
            lhs_active = lhs_active_next;
            lhs_i++;
        }
        // Event from rhs is coming first
        else if (rhs_t < lhs_t)
        {
            // Activate output
            if (!out_active && rhs_active_next)
            {
                out_active = true;
                out(out_i).start = rhs_t;
            }
            // Deactivate output
            else if (out_active && !lhs_active && !rhs_active_next)
            {
                out_active = false;
                out(out_i).stop = rhs_t;
                out_i++;
            }
          
            rhs_active = rhs_active_next;
            rhs_i++;
        }
        // Event from lhs and rhs coming at same time
        else
        {
            // Activate output
            if (!out_active && (lhs_active_next || rhs_active_next))
            {
                out_active = true;
                out(out_i).start = lhs_t;
            }
            // Deactivate output
            else if (out_active && !(lhs_active_next || rhs_active_next))
            {
                out_active = false;
                out(out_i).stop = lhs_t;
                out_i++;
            }
            
            lhs_active = lhs_active_next;
            rhs_active = rhs_active_next;
            lhs_i++; rhs_i++;
        }   
    }
   
    // Process any remaining lhs events
    while (lhs_i < lhs.size * 2)
    {
        bool lhs_active_next = lhs_i % 2 == 0;    
        int lhs_t = lhs_active_next ? lhs(lhs_i / 2).start : lhs(lhs_i / 2).stop;
        
        // Activate output
        if (!out_active && lhs_active_next)
        {
            out_active = true;
            out(out_i).start = lhs_t;
        }
        // Deactivate output
        else if (out_active && !lhs_active_next)
        {
            out_active = false;
            out(out_i).stop = lhs_t;
            out_i++;
        }
        
        lhs_active = lhs_active_next;
        lhs_i++;
    }
    
    // Process any remaining rhs events
    while (rhs_i < rhs.size * 2)
    {
        bool rhs_active_next = rhs_i % 2 == 0;    
        int rhs_t = rhs_active_next ? rhs(rhs_i / 2).start : rhs(rhs_i / 2).stop;
        
        // Activate output
        if (!out_active && rhs_active_next)
        {
            out_active = true;
            out(out_i).start = rhs_t;
        }
        // Deactivate output
        else if (out_active && !rhs_active_next)
        {
            out_active = false;
            out(out_i).stop = rhs_t;
            out_i++;
        }
        
        rhs_active = rhs_active_next;
        rhs_i++;
    }
    
    // Return number of ranges added to output
    return out_i;
}

// Process intersection operation on arrays of ranges.
// Assumes `out` is pre-allocated to be large enough 
// to store result. Returns the number of ranges 
// generated as output.
int ranges_intersection(
    slice1d<range> out,
    const slice1d<range> lhs,
    const slice1d<range> rhs)
{
    // Activation state of each list of ranges
    bool out_active = false;
    bool lhs_active = false;
    bool rhs_active = false;
    
    // Event index for each list of ranges
    int out_i = 0;
    int lhs_i = 0;
    int rhs_i = 0;
  
    // While both ranges have events to process
    while (lhs_i < lhs.size * 2 && rhs_i < rhs.size * 2)
    {
        // Are the next lhs, and rhs events active or inactive
        bool lhs_active_next = lhs_i % 2 == 0;
        bool rhs_active_next = rhs_i % 2 == 0;
        
        // Time of the next lhs, and rhs events
        int lhs_t = lhs_active_next ? lhs(lhs_i / 2).start : lhs(lhs_i / 2).stop;
        int rhs_t = rhs_active_next ? rhs(rhs_i / 2).start : rhs(rhs_i / 2).stop;
    
        // Event from lhs coming first
        if (lhs_t < rhs_t)
        {
            // Activate output
            if (!out_active && rhs_active && lhs_active_next)
            {
                out_active = true;
                out(out_i).start = lhs_t;
            }
            // Deactivate output
            else if (out_active && !lhs_active_next)
            {
                out_active = false;
                out(out_i).stop = lhs_t;
                out_i++;
            }
      
            lhs_active = lhs_active_next;
            lhs_i++;
        }
        // Event from rhs coming first
        else if (rhs_t < lhs_t)
        {
            // Activate output
            if (!out_active && lhs_active && rhs_active_next)
            {
                out_active = true;
                out(out_i).start = rhs_t;
            }
            // Deactivate output
            else if (out_active && !rhs_active_next)
            {
                out_active = false;
                out(out_i).stop = rhs_t;
                out_i++;
            }
      
            rhs_active = rhs_active_next;
            rhs_i++;
        }
        // Event from lhs and rhs coming at same time
        else
        {
            // Activate output
            if (!out_active && (lhs_active_next && rhs_active_next))
            {
                out_active = true;
                out(out_i).start = lhs_t;
            }
            // Deactivate output
            else if (out_active && (!lhs_active_next || !rhs_active_next))
            {
                out_active = false;
                out(out_i).stop = lhs_t;
                out_i++;
            }
      
            lhs_active = lhs_active_next;
            rhs_active = rhs_active_next;
            lhs_i++; rhs_i++;
        }   
    }
    
    // Return number of ranges added to output
    return out_i;
}

// Process difference operation on arrays of ranges.
// Assumes `out` is pre-allocated to be large enough 
// to store result. Returns the number of ranges 
// generated as output.
int ranges_difference(
    slice1d<range> out,
    const slice1d<range> lhs,
    const slice1d<range> rhs)
{
    // Activation state of each list of ranges
    bool out_active = false;
    bool lhs_active = false;
    bool rhs_active = false;
    
    // Event index for each list of ranges
    int out_i = 0;
    int lhs_i = 0;
    int rhs_i = 0;
    
    // While both ranges have events to process
    while (lhs_i < lhs.size * 2 && rhs_i < rhs.size * 2)
    {
        // Are the next lhs, and rhs events active or inactive
        bool lhs_active_next = lhs_i % 2 == 0;
        bool rhs_active_next = rhs_i % 2 == 0;
        
        // Time of the next lhs, and rhs events
        int lhs_t = lhs_active_next ? lhs(lhs_i / 2).start : lhs(lhs_i / 2).stop;
        int rhs_t = rhs_active_next ? rhs(rhs_i / 2).start : rhs(rhs_i / 2).stop;
    
        // Event coming from lhs first
        if (lhs_t < rhs_t)
        {
            // Activate output
            if (!out_active && !rhs_active && lhs_active_next)
            {
                out_active = true;
                out(out_i).start = lhs_t;
            }
            // Deactivate output
            else if (out_active && !lhs_active_next)
            {
                out_active = false;
                out(out_i).stop = lhs_t;
                out_i++;
            }
      
            lhs_active = lhs_active_next;
            lhs_i++;
        }
        // Event coming from rhs first
        else if (rhs_t < lhs_t)
        {
            // Activate output
            if (!out_active && lhs_active && !rhs_active_next)
            {
                out_active = true;
                out(out_i).start = rhs_t;
            }
            // Deactivate output
            else if (out_active && rhs_active_next)
            {
                out_active = false;
                out(out_i).stop = rhs_t;
                out_i++;
            }
            
            rhs_active = rhs_active_next;
            rhs_i++;
        }
        // Event from lhs and rhs coming at same time
        else
        {
            // Activate output
            if (!out_active && lhs_active_next && !rhs_active_next)
            {
                out_active = true;
                out(out_i).start = lhs_t;
            }
            // Deactivate output
            else if (out_active && rhs_active_next)
            {
                out_active = false;
                out(out_i).stop = lhs_t;
                out_i++;
            }
      
            lhs_active = lhs_active_next;
            rhs_active = rhs_active_next;
            lhs_i++; rhs_i++;
        }   
    }
    
    // Process any remaining lhs events
    while (lhs_i < lhs.size * 2)
    {
        bool lhs_active_next = lhs_i % 2 == 0;    
        int lhs_t = lhs_active_next ? lhs(lhs_i / 2).start : lhs(lhs_i / 2).stop;
        
        // Activate output
        if (!out_active && lhs_active_next)
        {
            out_active = true;
            out(out_i).start = lhs_t;
        }
        // Deactivate output
        else if (out_active && !lhs_active_next)
        {
            out_active = false;
            out(out_i).stop = lhs_t;
            out_i++;
        }
        
        lhs_active = lhs_active_next;
        lhs_i++;
    }
    
    // Return number of ranges added to output
    return out_i;
}

//--------------------------------------

struct range_set
{
    array1d<int>   anims;           // Sorted ids of all anims with ranges in set
    array1d<range> anims_subranges; // Slices of `ranges` array for each anim 
    array1d<range> ranges;          // Full list of all ranges for all animations 
};

void range_set_union(
    range_set& out, 
    const range_set& lhs, 
    const range_set& rhs)
{ 
    // Allocate potential maximum number of anims and ranges we might to output
    out.anims.resize(lhs.anims.size + rhs.anims.size);
    out.anims_subranges.resize(lhs.anims.size + rhs.anims.size);
    out.ranges.resize(lhs.ranges.size + rhs.ranges.size);
    
    // Anim index for each list of ranges
    int out_i = 0;
    int lhs_i = 0;
    int rhs_i = 0;
    
    // Output ranges index
    int ranges_i = 0;
    
    // While both sets have animations
    while (lhs_i < lhs.anims.size && rhs_i < rhs.anims.size)
    {
        // If animation from lhs is first
        if (lhs.anims(lhs_i) < rhs.anims(rhs_i))
        {
            // Append subranges to output
            slice1d<range> sranges = lhs.ranges.slice(lhs.anims_subranges(lhs_i));
            
            out.anims(out_i) = lhs.anims(lhs_i);
            out.anims_subranges(out_i) = { ranges_i, ranges_i + sranges.size };
            out.ranges.slice(ranges_i, ranges_i + sranges.size) = sranges;
            
            ranges_i+=sranges.size;
            out_i++;
            lhs_i++;
        }
        // If animation from rhs is first
        else if (rhs.anims(rhs_i) < lhs.anims(lhs_i))
        {
            // Append subranges to output
            slice1d<range> sranges = rhs.ranges.slice(rhs.anims_subranges(rhs_i));
            
            out.anims(out_i) = rhs.anims(rhs_i);
            out.anims_subranges(out_i) = { ranges_i, ranges_i + sranges.size };
            out.ranges.slice(ranges_i, ranges_i + sranges.size) = sranges;
            
            ranges_i+=sranges.size;
            out_i++;
            rhs_i++;
        }
        // If both contain the same animation
        else 
        {
            // Append union of subranges to output
            int nranges = ranges_union(
                out.ranges.slice_from(ranges_i),
                lhs.ranges.slice(lhs.anims_subranges(lhs_i)),
                rhs.ranges.slice(rhs.anims_subranges(rhs_i)));      
            
            out.anims(out_i) = lhs.anims(lhs_i);
            out.anims_subranges(out_i) = { ranges_i, ranges_i + nranges };
            
            ranges_i+=nranges;
            out_i++;
            lhs_i++; rhs_i++;
        }
    }
    
    // Process any remaining lhs animations
    while (lhs_i < lhs.anims.size)
    {
        // Append subranges to output
        slice1d<range> sranges = lhs.ranges.slice(lhs.anims_subranges(lhs_i));
        
        out.anims(out_i) = lhs.anims(lhs_i);
        out.anims_subranges(out_i) = { ranges_i, ranges_i + sranges.size };
        out.ranges.slice(ranges_i, ranges_i + sranges.size) = sranges;
        
        ranges_i+=sranges.size;
        out_i++;
        lhs_i++;
    }
   
    // Process any remaining rhs animations
    while (rhs_i < rhs.anims.size)
    {
        // Append subranges to output
        slice1d<range> sranges = rhs.ranges.slice(rhs.anims_subranges(rhs_i));
        
        out.anims(out_i) = rhs.anims(rhs_i);
        out.anims_subranges(out_i) = { ranges_i, ranges_i + sranges.size };
        out.ranges.slice(ranges_i, ranges_i + sranges.size) = sranges;
        
        ranges_i+=sranges.size;
        out_i++;
        rhs_i++;
    }
    
    // Resize output to match what was added
    out.anims.resize(out_i);
    out.anims_subranges.resize(out_i);
    out.ranges.resize(ranges_i);
}

void range_set_intersection(
    range_set& out, 
    const range_set& lhs, 
    const range_set& rhs)
{ 
    // Allocate potential maximum number of anims and ranges we might to output
    out.anims.resize(lhs.anims.size + rhs.anims.size);
    out.anims_subranges.resize(lhs.anims.size + rhs.anims.size);
    out.ranges.resize(lhs.ranges.size + rhs.ranges.size);
    
    // Anim index for each list of ranges
    int out_i = 0;
    int lhs_i = 0;
    int rhs_i = 0;
    
    // Output ranges index
    int ranges_i = 0;
    
    // While both sets have animations
    while (lhs_i < lhs.anims.size && rhs_i < rhs.anims.size)
    {
        // If animation is in lhs but not rhs skip
        if (lhs.anims(lhs_i) < rhs.anims(rhs_i))
        {
            lhs_i++;
        }
        // If animation is in rhs but not lhs skip
        else if (rhs.anims(rhs_i) < lhs.anims(lhs_i))
        {
            rhs_i++;
        }
        // If animation is in both lhs and rhs
        else 
        {   
            // Append intersection of subranges to output
            int nranges = ranges_intersection(
                out.ranges.slice_from(ranges_i),
                lhs.ranges.slice(lhs.anims_subranges(lhs_i)),
                rhs.ranges.slice(rhs.anims_subranges(rhs_i)));      
            
            out.anims(out_i) = lhs.anims(lhs_i);
            out.anims_subranges(out_i) = { ranges_i, ranges_i + nranges };
            
            ranges_i+=nranges;
            out_i++;
            lhs_i++; rhs_i++;
        }
    }
    
    // Resize output to match what was added
    out.anims.resize(out_i);
    out.anims_subranges.resize(out_i);
    out.ranges.resize(ranges_i);
}

void range_set_difference(
    range_set& out, 
    const range_set& lhs, 
    const range_set& rhs)
{ 
    // Allocate potential maximum number of anims and ranges we might to output
    out.anims.resize(lhs.anims.size + rhs.anims.size);
    out.anims_subranges.resize(lhs.anims.size + rhs.anims.size);
    out.ranges.resize(lhs.ranges.size + rhs.ranges.size);
    
    // Anim index for each list of ranges
    int out_i = 0;
    int lhs_i = 0;
    int rhs_i = 0;
    
    // Output ranges index
    int ranges_i = 0;
    
    // While both sets have animations
    while (lhs_i < lhs.anims.size && rhs_i < rhs.anims.size)
    {
        // If animation from lhs is first
        if (lhs.anims(lhs_i) < rhs.anims(rhs_i))
        {
            slice1d<range> sranges = lhs.ranges.slice(lhs.anims_subranges(lhs_i));
            
            out.anims(out_i) = lhs.anims(lhs_i);
            out.anims_subranges(out_i) = { ranges_i, ranges_i + sranges.size };
            out.ranges.slice(ranges_i, ranges_i + sranges.size) = sranges;
            
            ranges_i+=sranges.size;
            out_i++;
            lhs_i++;
        }
        // If animation is in rhs but not lhs skip
        else if (rhs.anims(rhs_i) < lhs.anims(lhs_i))
        {
            rhs_i++;
        }
        // If animation is in both lhs and rhs
        else
        {
            // Append difference of subranges to output
            int nranges = ranges_difference(
                out.ranges.slice_from(ranges_i),
                lhs.ranges.slice(lhs.anims_subranges(lhs_i)),
                rhs.ranges.slice(rhs.anims_subranges(rhs_i)));      
            
            out.anims(out_i) = lhs.anims(lhs_i);
            out.anims_subranges(out_i) = { ranges_i, ranges_i + nranges };
            
            ranges_i+=nranges;
            out_i++;
            lhs_i++; rhs_i++;
        }
    }
    
    // Process any remaining lhs animations
    while (lhs_i < lhs.anims.size)
    {
        slice1d<range> sranges = lhs.ranges.slice(lhs.anims_subranges(lhs_i));
        
        out.anims(out_i) = lhs.anims(lhs_i);
        out.anims_subranges(out_i) = { ranges_i, ranges_i + sranges.size };
        out.ranges.slice(ranges_i, ranges_i + sranges.size) = sranges;
        
        ranges_i+=sranges.size;
        out_i++;
        lhs_i++;
    }
    
    // Resize output to match what was added
    out.anims.resize(out_i);
    out.anims_subranges.resize(out_i);
    out.ranges.resize(ranges_i);
}

//--------------------------------------

void mask_union(
    slice1d_bit out,
    const slice1d_bit lhs,
    const slice1d_bit rhs)
{
    assert((out.size == lhs.size) && (out.size == rhs.size));
    
    for (int i = 0; i < out.size; i++)
    {
        out.set(i, lhs.get(i) || rhs.get(i));
    }
}

void mask_intersection(
    slice1d_bit out,
    const slice1d_bit lhs,
    const slice1d_bit rhs)
{
    assert((out.size == lhs.size) && (out.size == rhs.size));
    
    for (int i = 0; i < out.size; i++)
    {
        out.set(i, lhs.get(i) && rhs.get(i));
    }
}

void mask_difference(
    slice1d_bit out,
    const slice1d_bit lhs,
    const slice1d_bit rhs)
{
    assert((out.size == lhs.size) && (out.size == rhs.size));
    
    for (int i = 0; i < out.size; i++)
    {
        out.set(i, lhs.get(i) && !rhs.get(i));
    }
}

void mask_custom_logic(
    slice1d_bit query,
    const slice1d_bit running,
    const slice1d_bit male,
    const slice1d_bit tired,
    const slice1d_bit limping)
{   
    for (int i = 0; i < query.size; i++)
    {
        query.set(i,
            running.get(i) && male.get(i) && (tired.get(i) || limping.get(i)));
    }
}

//--------------------------------------

struct mask_set
{
    array1d<int>   anims;           // Sorted ids of all anims with masks in set
    array1d<range> anims_submasks;  // Slices of `masks` array for each anim
    array1d_bit    masks;           // Full list of all masks for all animations 
};

void mask_set_union(
    mask_set& out, 
    const mask_set& lhs, 
    const mask_set& rhs)
{ 
    // Allocate potential maximum number of anims and ranges we might to output
    out.anims.resize(lhs.anims.size + rhs.anims.size);
    out.anims_submasks.resize(lhs.anims.size + rhs.anims.size);
    out.masks.resize(lhs.masks.size + rhs.masks.size);
    
    // Anim index for each list of masks
    int out_i = 0;
    int lhs_i = 0;
    int rhs_i = 0;
    
    // Output masks index
    int masks_i = 0;
    
    // While both sets have animations
    while (lhs_i < lhs.anims.size && rhs_i < rhs.anims.size)
    {
        // If animation from lhs is first
        if (lhs.anims(lhs_i) < rhs.anims(rhs_i))
        {
            // Append submask to output
            slice1d_bit submask = lhs.masks.slice(lhs.anims_submasks(lhs_i));
            
            out.anims(out_i) = lhs.anims(lhs_i);
            out.anims_submasks(out_i) = { masks_i, masks_i + submask.size };
            out.masks.slice(masks_i, masks_i + submask.size) = submask;            
            
            masks_i+=submask.size;
            out_i++;
            lhs_i++;
        }
        // If animation from rhs is first
        else if (rhs.anims(rhs_i) < lhs.anims(lhs_i))
        {
            // Append submask to output
            slice1d_bit submask = rhs.masks.slice(rhs.anims_submasks(rhs_i));
            
            out.anims(out_i) = rhs.anims(rhs_i);
            out.anims_submasks(out_i) = { masks_i, masks_i + submask.size };
            out.masks.slice(masks_i, masks_i + submask.size) = submask;
            
            masks_i+=submask.size;
            out_i++;
            rhs_i++;
        }
        // If both contain the same animation
        else 
        {
            // Append union of submask to output
            int nmasks = lhs.masks.slice(lhs.anims_submasks(lhs_i)).size;
            
            mask_union(
                out.masks.slice(masks_i, masks_i + nmasks),
                lhs.masks.slice(lhs.anims_submasks(lhs_i)),
                rhs.masks.slice(rhs.anims_submasks(rhs_i)));      
            
            out.anims(out_i) = lhs.anims(lhs_i);
            out.anims_submasks(out_i) = { masks_i, masks_i + nmasks };
            
            masks_i+=nmasks;
            out_i++;
            lhs_i++; rhs_i++;
        }
    }
    
    // Process any remaining lhs animations
    while (lhs_i < lhs.anims.size)
    {
        // Append submask to output
        slice1d_bit submask = lhs.masks.slice(lhs.anims_submasks(lhs_i));
        
        out.anims(out_i) = lhs.anims(lhs_i);
        out.anims_submasks(out_i) = { masks_i, masks_i + submask.size };
        out.masks.slice(masks_i, masks_i + submask.size) = submask;
        
        masks_i+=submask.size;
        out_i++;
        lhs_i++;
    }
   
    // Process any remaining rhs animations
    while (rhs_i < rhs.anims.size)
    {
        // Append submask to output
        slice1d_bit submask = rhs.masks.slice(rhs.anims_submasks(rhs_i));
        
        out.anims(out_i) = rhs.anims(rhs_i);
        out.anims_submasks(out_i) = { masks_i, masks_i + submask.size };
        out.masks.slice(masks_i, masks_i + submask.size) = submask;
        
        masks_i+=submask.size;
        out_i++;
        rhs_i++;
    }
    
    // Resize output to match what was added
    out.anims.resize(out_i);
    out.anims_submasks.resize(out_i);
    out.masks.resize(masks_i);
}

void mask_set_intersection(
    mask_set& out, 
    const mask_set& lhs, 
    const mask_set& rhs)
{ 
    // Allocate potential maximum number of anims and masks we might to output
    out.anims.resize(lhs.anims.size + rhs.anims.size);
    out.anims_submasks.resize(lhs.anims.size + rhs.anims.size);
    out.masks.resize(lhs.masks.size + rhs.masks.size);
    
    // Anim index for each list of masks
    int out_i = 0;
    int lhs_i = 0;
    int rhs_i = 0;
    
    // Output masks index
    int masks_i = 0;
    
    // While both sets have animations
    while (lhs_i < lhs.anims.size && rhs_i < rhs.anims.size)
    {
        // If animation is in lhs but not rhs skip
        if (lhs.anims(lhs_i) < rhs.anims(rhs_i))
        {
            lhs_i++;
        }
        // If animation is in rhs but not lhs skip
        else if (rhs.anims(rhs_i) < lhs.anims(lhs_i))
        {
            rhs_i++;
        }
        // If animation is in both lhs and rhs
        else 
        {   
            // Append intersection of submask to output
            int nmasks = lhs.masks.slice(lhs.anims_submasks(lhs_i)).size;
            
            mask_intersection(
                out.masks.slice(masks_i, masks_i + nmasks),
                lhs.masks.slice(lhs.anims_submasks(lhs_i)),
                rhs.masks.slice(rhs.anims_submasks(rhs_i)));      
            
            out.anims(out_i) = lhs.anims(lhs_i);
            out.anims_submasks(out_i) = { masks_i, masks_i + nmasks };
            
            masks_i+=nmasks;
            out_i++;
            lhs_i++; rhs_i++;
        }
    }
    
    // Resize output to match what was added
    out.anims.resize(out_i);
    out.anims_submasks.resize(out_i);
    out.masks.resize(masks_i);
}

void mask_set_difference(
    mask_set& out, 
    const mask_set& lhs, 
    const mask_set& rhs)
{ 
    // Allocate potential maximum number of anims and ranges we might to output
    out.anims.resize(lhs.anims.size + rhs.anims.size);
    out.anims_submasks.resize(lhs.anims.size + rhs.anims.size);
    out.masks.resize(lhs.masks.size + rhs.masks.size);
    
    // Anim index for each list of masks
    int out_i = 0;
    int lhs_i = 0;
    int rhs_i = 0;
    
    // Output masks index
    int masks_i = 0;
    
    // While both sets have animations
    while (lhs_i < lhs.anims.size && rhs_i < rhs.anims.size)
    {
        // If animation from lhs is first
        if (lhs.anims(lhs_i) < rhs.anims(rhs_i))
        {
            slice1d_bit submask = lhs.masks.slice(lhs.anims_submasks(lhs_i));
            
            out.anims(out_i) = lhs.anims(lhs_i);
            out.anims_submasks(out_i) = { masks_i, masks_i + submask.size };
            out.masks.slice(masks_i, masks_i + submask.size) = submask;
            
            masks_i+=submask.size;
            out_i++;
            lhs_i++;
        }
        // If animation is in rhs but not lhs skip
        else if (rhs.anims(rhs_i) < lhs.anims(lhs_i))
        {
            rhs_i++;
        }
        // If animation is in both lhs and rhs
        else
        {
            // Append difference of submask to output
            int nmasks = lhs.masks.slice(lhs.anims_submasks(lhs_i)).size;
            
            mask_difference(
                out.masks.slice(masks_i, masks_i + nmasks),
                lhs.masks.slice(lhs.anims_submasks(lhs_i)),
                rhs.masks.slice(rhs.anims_submasks(rhs_i)));      
            
            out.anims(out_i) = lhs.anims(lhs_i);
            out.anims_submasks(out_i) = { masks_i, masks_i + nmasks };
            
            masks_i+=nmasks;
            out_i++;
            lhs_i++; rhs_i++;
        }
    }
    
    // Process any remaining lhs animations
    while (lhs_i < lhs.anims.size)
    {
        slice1d_bit submask = lhs.masks.slice(lhs.anims_submasks(lhs_i));
        
        out.anims(out_i) = lhs.anims(lhs_i);
        out.anims_submasks(out_i) = { masks_i, masks_i + submask.size };
        out.masks.slice(masks_i, masks_i + submask.size) = submask;
        
        masks_i+=submask.size;
        out_i++;
        lhs_i++;
    }
    
    // Resize output to match what was added
    out.anims.resize(out_i);
    out.anims_submasks.resize(out_i);
    out.masks.resize(masks_i);
}


//--------------------------------------

// Query operations encoded by negative numbers
enum
{
    QUERY_OP_UNION        = -1,
    QUERY_OP_INTERSECTION = -2,
    QUERY_OP_DIFFERENCE   = -3
};

// Query expr object consists of a stack of set 
// indices and logical operations
struct query_expr
{
    // Construct empty expression
    query_expr() {}
    
    // Construct from a single set index
    query_expr(int set) : stack(1) { stack(0) = set; }
    
    // Construct from two other range set queries and an op
    query_expr(
        const query_expr& lhs,
        const query_expr& rhs,
        int op)
        : stack(lhs.stack.size + rhs.stack.size + 1)
    {
        // Assert op is negative
        assert(op < 0);
        
        // Copy rhs and lhs into stack
        stack.slice(0, rhs.stack.size) = rhs.stack;
        stack.slice(rhs.stack.size, rhs.stack.size + lhs.stack.size) = lhs.stack;
        
        // Put op on top
        stack(stack.size - 1) = op;
    }
    
    // Inplace array used to store stack - can store
    // up to 16 elements without using heap allocation
    inplace_array1d<int, 16> stack;
};

query_expr operator|(const query_expr& lhs, const query_expr& rhs)
{
    return query_expr(lhs, rhs, QUERY_OP_UNION);
}

query_expr operator&(const query_expr& lhs, const query_expr& rhs)
{
    return query_expr(lhs, rhs, QUERY_OP_INTERSECTION);
}

query_expr operator-(const query_expr& lhs, const query_expr& rhs)
{
    return query_expr(lhs, rhs, QUERY_OP_DIFFERENCE);
}

//--------------------------------------

// Quick and dirty hash function
size_t memhash(const void* ptr, size_t num)
{
    size_t h = 0x7A99ED;
    for (int i = 0; i < num; i++)
    {
        h = (h * 54059) ^ (((const char*)ptr)[i] * 76963);
    }
    return h;
}

// Hash function for queries
struct query_expr_hash
{
    size_t operator()(const query_expr& x) const
    {
        return memhash(x.stack.data, sizeof(int) * x.stack.size);
    }
};

// Comparison function for queries
struct query_expr_cmp
{
    bool operator()(const query_expr& lhs, const query_expr& rhs) const
    {   
        if (lhs.stack.size == rhs.stack.size)
        {
            return memcmp(
                lhs.stack.data, 
                rhs.stack.data, 
                sizeof(int) * lhs.stack.size) == 0;
        }
        else
        {
            return false;
        }
    }
};

// Hashtable to cache range set evaluations
using query_expr_range_set_cache = std::unordered_map<
    query_expr,
    range_set,
    query_expr_hash,
    query_expr_cmp>;

// Hashtable to cache mask set evaluations
using query_expr_mask_set_cache = std::unordered_map<
    query_expr,
    mask_set,
    query_expr_hash,
    query_expr_cmp>;

//--------------------------------------

// Recursively evaluate range set query by
// walking down the stack from top to bottom 
// and performing the operations encoded by them
void query_expr_evaluate_range_set_from(
    range_set& out,
    int& index,
    const query_expr& query, 
    const std::vector<range_set>& range_sets)
{   
    range_set lhs, rhs;

    switch (query.stack(index))
    {
        case QUERY_OP_UNION:
            index--;
            query_expr_evaluate_range_set_from(lhs, index, query, range_sets);
            query_expr_evaluate_range_set_from(rhs, index, query, range_sets);
            range_set_union(out, lhs, rhs);  
        break;
        
        case QUERY_OP_INTERSECTION:
            index--;
            query_expr_evaluate_range_set_from(lhs, index, query, range_sets);
            query_expr_evaluate_range_set_from(rhs, index, query, range_sets);
            range_set_intersection(out, lhs, rhs);
        break;
        
        case QUERY_OP_DIFFERENCE:
            index--;
            query_expr_evaluate_range_set_from(lhs, index, query, range_sets);
            query_expr_evaluate_range_set_from(rhs, index, query, range_sets);
            range_set_difference(out, lhs, rhs);
        break;
      
        default:
            out = range_sets[query.stack(index)];
            index--;
        break;
    }
}

void query_expr_evaluate_range_set(
    range_set& out,
    const query_expr& query, 
    const std::vector<range_set>& range_sets)
{ 
    // If empty query, return empty range set
    if (query.stack.size == 0)
    {
        out = range_set();
    }
    else
    {
        // Start at top of stack and evaluate
        int index = query.stack.size - 1;
        query_expr_evaluate_range_set_from(out, index, query, range_sets);
        
        // Assert we've consumed all of the stack
        assert(index == -1);
    }
}

//--------------------------------------

void query_expr_evaluate_mask_set_from(
    mask_set& out,
    int& index,
    const query_expr& query, 
    const std::vector<mask_set>& mask_sets)
{   
    mask_set lhs, rhs;

    switch (query.stack(index))
    {
        case QUERY_OP_UNION:
            index--;
            query_expr_evaluate_mask_set_from(lhs, index, query, mask_sets);
            query_expr_evaluate_mask_set_from(rhs, index, query, mask_sets);
            mask_set_union(out, lhs, rhs);  
        break;
        
        case QUERY_OP_INTERSECTION:
            index--;
            query_expr_evaluate_mask_set_from(lhs, index, query, mask_sets);
            query_expr_evaluate_mask_set_from(rhs, index, query, mask_sets);
            mask_set_intersection(out, lhs, rhs);
        break;
        
        case QUERY_OP_DIFFERENCE:
            index--;
            query_expr_evaluate_mask_set_from(lhs, index, query, mask_sets);
            query_expr_evaluate_mask_set_from(rhs, index, query, mask_sets);
            mask_set_difference(out, lhs, rhs);
        break;
      
        default:
            out = mask_sets[query.stack(index)];
            index--;
        break;
    }
}

void query_expr_evaluate_mask_set(
    mask_set& out,
    const query_expr& query, 
    const std::vector<mask_set>& mask_sets)
{ 
    if (query.stack.size == 0)
    {
        out = mask_set();
    }
    else
    {
        int index = query.stack.size - 1;
        query_expr_evaluate_mask_set_from(out, index, query, mask_sets);
        
        assert(index == -1);
    }
}

//--------------------------------------

void ranges_rasterize(
    slice1d_bit out,
    const slice1d<range> ranges)
{
    // Zero out all bits
    out.zero();
    
    for (int i = 0; i < ranges.size; i++)
    {
        // Write ones for range
        out.slice(ranges(i)).one();
    }
}

void range_set_rasterize(
    mask_set& out,
    const range_set& set,
    const range_set& set_all)
{
    out.anims = set.anims;
    out.anims_submasks.resize(set.anims.size);
    
    // First find out the size of each mask
    int masks_i = 0;
    for (int i = 0; i < out.anims.size; i++)
    {
        int nmasks = set_all.ranges(out.anims(i)).stop - set_all.ranges(out.anims(i)).start;
        
        out.anims_submasks(i) = { masks_i, masks_i + nmasks };
        
        masks_i += nmasks;
    }
    
    // Then go ahead and rasterize those masks
    out.masks.resize(masks_i);
    for (int i = 0; i < set.anims.size; i++)
    {
        ranges_rasterize(
            out.masks.slice(out.anims_submasks(i)), 
            set.ranges.slice(set.anims_subranges(i)));
    }
}

// Converts a mask into a set of ranges, assumes
// `out` is pre-allocated to be large enough to
// store result. Returns number of ranges added.
int mask_vectorize(
    slice1d<range> out,
    const slice1d_bit mask)
{
    bool out_active = false;
    int out_i = 0;

    for (int i = 0; i < mask.size; i++)
    {
        // Activate output
        if (!out_active && mask.get(i))
        {
            out(out_i).start = i;
            out_active = true;
        }
        // Deactivate output
        else if (out_active && !mask.get(i))
        {
            out(out_i).stop = i;
            out_active = false;
            out_i++;
        }
    }
    
    // Deactivate output
    if (out_active)
    {
        out(out_i).stop = mask.size;
        out_i++;
    }
    
    return out_i;
}

// Counts number of ranges added
int mask_vectorize_count(const slice1d_bit mask)
{
    bool out_active = false;

    int out_i = 0;

    for (int i = 0; i < mask.size; i++)
    {
        if (!out_active && mask.get(i))
        {
            out_active = true;
        }
        else if (out_active && !mask.get(i))
        {
            out_active = false;
            out_i++;
        }
    }
    
    if (out_active)
    {
        out_i++;        
    }
    
    return out_i;
}

void mask_set_vectorize(
    range_set& out,
    const mask_set& set)
{
    out.anims = set.anims;
    out.anims_subranges.resize(set.anims.size);
    
    // Find out number of ranges to be added
    int ranges_i = 0;
    for (int i = 0; i < set.anims.size; i++)
    {
        int nranges = mask_vectorize_count(set.masks.slice(set.anims_submasks(i)));
        
        out.anims_subranges(i) = { ranges_i, ranges_i + nranges };
        
        ranges_i += nranges;
    }
    
    // Add those ranges
    out.ranges.resize(ranges_i);
    for (int i = 0; i < out.anims.size; i++)
    { 
        mask_vectorize(
            out.ranges.slice(out.anims_subranges(i)),
            set.masks.slice(set.anims_submasks(i)));
    }
}

//--------------------------------------

// The below are some quick and messy
// functions for parsing the user input 
// string. Essentially they either move 
// the input pointer `i` forward and 
// return a expression in `query` or 
// they return an error in `err`

void query_expr_parse_union(
  int& i, 
  query_expr& query,
  char* err,
  const std::vector<std::string> tag_names, 
  const char* query_string);

void query_expr_parse_identifier(
  int& i, 
  query_expr& query,
  char* err,
  const std::vector<std::string> tag_names, 
  const char* query_string)
{
    std::string identifier;
    
    while (isalnum(query_string[i]))
    {
        identifier += query_string[i];
        i++;
    }
    
    auto it = std::find(tag_names.begin(), tag_names.end(), identifier);
    
    if (it != tag_names.end())
    {
        query = query_expr(it - tag_names.begin());
        return;
    }
    else
    {
        sprintf(err, "Unknown tag name: \"%s\"\n", identifier.c_str());
        return;
    }
}

void query_expr_parse_expression(
    int& i, 
    query_expr& query,
    char* err,
    const std::vector<std::string> tag_names, 
    const char* query_string)
{
    while (isspace(query_string[i])) { i++; }
    
    if (query_string[i] == '(')
    {
        i++;
        
        query_expr_parse_union(
            i,
            query,
            err,
            tag_names,
            query_string);
            
        if (strlen(err)) { return; }
        
        while (isspace(query_string[i])) { i++; }
        
        if (query_string[i] != ')')
        {
            sprintf(err, "Unmatched parenthesis\n");
            return;
        }
        
        i++;
        
        return;
    }
    else if (isalnum(query_string[i]))
    {
        query_expr_parse_identifier(
            i, query, err, tag_names, query_string);
        return;
    }
    else if (query_string[i] == '\0')
    {
        sprintf(err, "Unexpected end of input\n");
        return;
    }
    else
    {
        sprintf(err, "Syntax Error at '%c'\n", query_string[i]);
        return;
    }
}

void query_expr_parse_difference(
  int& i, 
  query_expr& query,
  char* err,
  const std::vector<std::string> tag_names, 
  const char* query_string)
{
    query_expr_parse_expression(
      i,
      query,
      err,
      tag_names,
      query_string);
    
    if (strlen(err)) { return; }
    
    while (isspace(query_string[i])) { i++; }
    
    while (query_string[i] == '-')
    {   
        i++;
        
        query_expr expr;
        
        query_expr_parse_expression(
          i,
          expr,
          err,
          tag_names,
          query_string);
        
        if (strlen(err)) { return; }
        
        query = query - expr;
      
        while (isspace(query_string[i])) { i++; }
    }
}

void query_expr_parse_intersection(
  int& i, 
  query_expr& query,
  char* err,
  const std::vector<std::string> tag_names, 
  const char* query_string)
{
    query_expr_parse_difference(
      i,
      query,
      err,
      tag_names,
      query_string);
  
    if (strlen(err)) { return; }
    
    while (isspace(query_string[i])) { i++; }
    
    while (query_string[i] == '&')
    {
        i++;
        
        query_expr diff;
        
        query_expr_parse_difference(
          i,
          diff,
          err,
          tag_names,
          query_string);
        
        if (strlen(err)) { return; }
        
        query = query & diff;
        
        while (isspace(query_string[i])) { i++; }
    }
}

void query_expr_parse_union(
  int& i, 
  query_expr& query,
  char* err,
  const std::vector<std::string> tag_names, 
  const char* query_string)
{
    query_expr_parse_intersection(
      i,
      query,
      err,
      tag_names,
      query_string);
  
    if (strlen(err)) { return; }
    
    while (isspace(query_string[i])) { i++; }
    
    while (query_string[i] == '|')
    {   
        i++;
  
        query_expr inter;
        
        query_expr_parse_intersection(
          i,
          inter,
          err,
          tag_names,
          query_string);
        
        if (strlen(err)) { return; }
        
        query = query | inter;
        
        while (isspace(query_string[i])) { i++; }
    }
}

//--------------------------------------

// These functions are for parsing the tag data from 
// the ascii representation embedded in this file

int parse_tag_data_tag_name(
    int i,
    std::vector<std::string>& tag_names,
    std::vector<range_set>& tag_range_sets,
    const char* tag_data_string)
{
    tag_names.push_back("");
    tag_range_sets.push_back(range_set());
    
    while (tag_data_string[i] != '|')
    {
        if (isascii(tag_data_string[i]) && !isspace(tag_data_string[i]))
        {
            tag_names.back() += tag_data_string[i];
        }
        i++;
    }
    
    return i;
}

int parse_tag_data_anim(
    int i,
    int anim_id,
    range_set& set,
    const char* tag_data_string)
{
    while (tag_data_string[i] != '|') { i++; }
    i++;
    
    int offset = i;
    bool active = false;
    
    array1d<range> tag_ranges;
    
    while (tag_data_string[i] != '|')
    {
        if (!active && (tag_data_string[i] == '#'))
        {
            tag_ranges.resize(tag_ranges.size + 1);
            tag_ranges(tag_ranges.size - 1).start = i - offset;
            active = true;
        }
        else if (active && !(tag_data_string[i] == '#'))
        {
            tag_ranges(tag_ranges.size -  1).stop = i - offset;
            active = false;
        }
        
        i++;
    }
    
    if (active)
    {
        tag_ranges(tag_ranges.size -  1).stop = i - offset;
        active = false;
    }
    
    if (tag_ranges.size != 0)
    { 
        int new_anim_size = set.anims.size + 1;
        int old_range_size = set.ranges.size;
        int new_range_size = old_range_size + tag_ranges.size;

        set.anims.resize(new_anim_size);
        set.anims_subranges.resize(new_anim_size);
        set.ranges.resize(new_range_size);
        
        set.anims(new_anim_size - 1) = anim_id;
        set.anims_subranges(new_anim_size - 1) = { old_range_size, new_range_size };        
        set.ranges.slice(old_range_size, new_range_size) = tag_ranges;
    }
    
    i++;
    
    return i;
}

void parse_tag_data(
    std::vector<std::string>& tag_names,
    std::vector<range_set>& tag_range_sets,
    const char* tag_data_string)
{
    int i = 0;
    while (tag_data_string[i] != '\0')
    {   
        i = parse_tag_data_tag_name(i, tag_names, tag_range_sets, tag_data_string);
        
        int anim_id = 0;
        while (tag_data_string[i] != '\n')
        {
            i = parse_tag_data_anim(i, anim_id, tag_range_sets.back(), tag_data_string);
            anim_id++;
        }
        
        i++;
    }
}

int tag_index(const std::vector<std::string>& tag_names, std::string name)
{
    auto it = std::find(tag_names.begin(), tag_names.end(), name);
    
    if (it != tag_names.end())
    {
        return it - tag_names.begin();
    }
    
    assert(false);
    return -1;
}

//--------------------------------------

int index_of(slice1d<int> x, int y)
{
    for (int i = 0; i < x.size; i++)
    {
        if (x(i) == y) return i;
    }
    return -1;
}

void draw_anim_names(
    const range_set& all_tag_range_set,
    int height,
    int scale = 8)
{
    int padding = 10;
    
    int offset = 150;
    
    for (int i = 0; i < all_tag_range_set.anims.size; i++)
    {   
        DrawText(TextFormat("Anim %i", i), 
          offset + scale * (all_tag_range_set.ranges(i).stop / 2) - 20, height, 20, DARKGRAY);
        
        offset += scale * all_tag_range_set.ranges(i).stop + padding;
    }
    
}

void draw_tag_range_set(
    const char* tag_name,
    const range_set& tag_range_set,
    const range_set& all_tag_range_set,
    int height,
    Color color,
    int scale = 8)
{
    DrawText(tag_name, 20, height, 20, DARKGRAY);
    
    int padding = 10;
    
    int offset = 150;

    for (int i = 0; i < all_tag_range_set.anims.size; i++)
    {   
        int j = index_of(tag_range_set.anims, i);
        if (j != -1)
        {
            int start = tag_range_set.anims_subranges(j).start;
            int stop = tag_range_set.anims_subranges(j).stop;
            for (int k = start; k < stop; k++)
            {
                Rectangle rec = {
                  (float)offset + scale * tag_range_set.ranges(k).start + 2, 
                  (float)height, 
                  (float)scale * (tag_range_set.ranges(k).stop - tag_range_set.ranges(k).start) - 4, 
                  (float)20,
                };
                  
                DrawRectangleRounded(rec, 0.1f, 0, Fade(color, 0.4f));
                DrawRectangleRoundedLines(rec, 0.1f, 0, 2.0f, Fade(color, 0.8f));
            }
        }
        
        offset += scale * all_tag_range_set.ranges(i).stop + padding;
    }
    
    offset = 150;
    
    for (int i = 0; i < all_tag_range_set.anims.size; i++)
    {   
        DrawLine(offset - 1, height - 3, offset - 1, height + 23, GRAY);
        offset += scale * all_tag_range_set.ranges(i).stop;
        DrawLine(offset + 1, height - 3, offset + 1, height + 23, GRAY);        
        offset += padding;
    }
    
}

//--------------------------------------

void update_callback(void* args)
{
    ((std::function<void()>*)args)->operator()();
}

//--------------------------------------

int main(void)
{
    // Should we use text input or a hard-coded query
    bool use_hardcoded = false;

    std::vector<std::string> tag_names;
    std::vector<range_set> tag_range_sets;
    std::vector<mask_set> tag_mask_sets;
    
    // Some different taggings you can try
    
    const char* tag_data_string = 
        "All        |##################################| |#####################| |#######################| |#############################| |######################|\n"
        "None       |                                  | |                     | |                       | |                             | |                      |\n"
        "Male       |##################################| |#####################| |                       | |                             | |                      |\n"
        "Female     |                                  | |                     | |#######################| |#############################| |######################|\n"
        "Junk       |##                       ##    ###| |##                ###| |##                 ####| |#####                ########| |####               ###|\n"
        "TPose      |##                              ##| |##                 ##| |##                   ##| |###                       ###| |##                  ##|\n"
        "Locomotion |  #############################   | |  ###############    | |  #################    | |     #####################   | |    ###############   |\n"
        "Transition |        ####             ##       | |    ##    ##         | |            ###        | |         ##          ##      | |        ####          |\n"
        "Walk       |  ######                   ####   | |  ##        #####    | |               ####    | |     ####              ###   | |    ####    #######   |\n" 
        "WalkToRun  |        ####                      | |    ##               | |                       | |         ##                  | |        ##            |\n"
        "Run        |            #############         | |      ####           | |  ##########           | |           ##########        | |                      |\n"
        "RunToWalk  |                         ##       | |          ##         | |            ###        | |                     ##      | |          ##          |\n"
        "Tired      |                    #####         | |                     | |         ###           | |                             | |                      |\n"
        "Injured    |                                  | |                     | |                       | |                 ####        | |                ###   |\n"
        "Preferred  |             ######               | |             ##      | |   ####                | |                             | |    ###               |\n"
    ;
    
    const char* tag_data_string_alt = 
        "All        |###############################| |#############################|\n"
        "None       |                               | |                             |\n"
        "Male       |###############################| |                             |\n"
        "Female     |                               | |#############################|\n"
        "Running    |####################           | |#############                |\n"
        "Walking    |                    ###########| |             ################|\n"
        "Tired      |           ######              | |         ####################|\n"
        "Limping    |                 ##############| |                       ######|\n"
    ;
    
    const char* tag_data_string_simple = 
        "All        |###############################################################|\n"
        "None       |                                                               |\n"
        "Ranges0    |######################         ########               #########|\n"
        "Ranges1    |        ##################                #########   #########|\n"
    ;
    
    // Parse tag data
    
    parse_tag_data(
        tag_names,
        tag_range_sets,
        use_hardcoded ? tag_data_string_alt : tag_data_string);
    
    // Convert to masks
    
    tag_mask_sets.resize(tag_range_sets.size());
    
    for (int i = 0; i < tag_range_sets.size(); i++)
    {
        range_set_rasterize(
            tag_mask_sets[i],
            tag_range_sets[i],
            tag_range_sets[0]);
    }
    
    // Caches
    
    query_expr_range_set_cache range_cache;
    query_expr_mask_set_cache mask_cache;
    
    // Init Window
    
    const int screen_width = 1280;
    const int screen_height = 720;
    
    SetConfigFlags(FLAG_VSYNC_HINT);
    SetConfigFlags(FLAG_MSAA_4X_HINT);
    InitWindow(screen_width, screen_height, "raylib [ranges]");
    SetTargetFPS(60);
    
    // UI
    
    const Color colors[] = {
        YELLOW,
        GOLD,
        ORANGE,
        PINK,
        RED,
        MAROON,
        GREEN,
        LIME,
        DARKGREEN,
        SKYBLUE,
        BLUE,
        DARKBLUE,
        PURPLE,
        VIOLET,
        DARKPURPLE,
        BEIGE,
        BROWN,
        DARKBROWN,
    };
    
    char query_buffer[1024];
    char error_buffer[1024];
    
    query_buffer[0] = '\0';
    error_buffer[0] = '\0';
    
    // Should we use masks to do the query?
    bool use_masks = false;
    
    // Go

    auto update_func = [&]()
    {
        BeginDrawing();

        ClearBackground(RAYWHITE);
        
        int height = 70;
        int scale = use_hardcoded ? 17 : 8;
        int linelength = use_hardcoded ? 1180 : 1220;
        
        draw_anim_names(tag_range_sets[0], 20, scale);

        DrawLine(152, height - 10, linelength, height - 10, GRAY);        
        
        for (int i = use_hardcoded ? 2 : 0; i < tag_names.size(); i++)
        {
            draw_tag_range_set(
              tag_names[i].c_str(), 
              tag_range_sets[i], 
              tag_range_sets[0], 
              height, 
              colors[i],
              scale);
              
            height += 30;
        }
      
        DrawLine(152, height, linelength, height, GRAY);
        
        /* Parse and evaluate query */
        
        error_buffer[0] = '\0';

        query_expr query;
        range_set query_range_set;
        mask_set query_mask_set;
        
        if (use_hardcoded)
        {
            query_expr Male(tag_index(tag_names, "Male"));
            query_expr Female(tag_index(tag_names, "Female"));
            query_expr Running(tag_index(tag_names, "Running"));
            query_expr Walking(tag_index(tag_names, "Walking"));
            query_expr Tired(tag_index(tag_names, "Tired"));
            query_expr Limping(tag_index(tag_names, "Limping"));
            
            query = Running & Male & (Tired | Limping);
            
            query_expr_evaluate_range_set(query_range_set, query, tag_range_sets);
        }
        else
        {
            if (strlen(query_buffer))
            {
                int i = 0;
                query_expr_parse_union(
                    i,
                    query,
                    error_buffer,
                    tag_names,
                    query_buffer);
                
                if (strlen(error_buffer))
                {   
                    query_range_set = tag_range_sets[1];
                }
                else
                {   
                    if (use_masks)
                    {
                        auto lookup = mask_cache.find(query);
                  
                        if (lookup != mask_cache.end())
                        {
                            query_mask_set = lookup->second;
                        }
                        else
                        {
                            query_expr_evaluate_mask_set(query_mask_set, query, tag_mask_sets);
                            mask_cache[query] = query_mask_set;
                        }
                        
                        mask_set_vectorize(
                            query_range_set,
                            query_mask_set);
                    }
                    else
                    {
                        auto lookup = range_cache.find(query);
                  
                        if (lookup != range_cache.end())
                        {
                            query_range_set = lookup->second;
                        }
                        else
                        {
                            query_expr_evaluate_range_set(query_range_set, query, tag_range_sets);
                            range_cache[query] = query_range_set;
                        }
                    }
                }
            }
            else
            {
                query_range_set = tag_range_sets[1];
            }
        }
        
        // Draw Query
        
        height += 10;

        draw_tag_range_set(
          "Query", 
          query_range_set, 
          tag_range_sets[0], 
          height, 
          LIGHTGRAY,
          scale);
          
        height += 30;

        // Draw Text Box

        DrawLine(152, height, linelength, height, GRAY);
        
        if (use_hardcoded)
        {
            DrawText("Running & Male & (Tired | Limping)", linelength / 2 - 100, height + 30, 20, DARKGRAY);
        }
        else
        {
            GuiTextBox((Rectangle){
                380, (float)height + 30, 600, 50
            }, query_buffer, 50, true);
            
            if (strlen(error_buffer))
            {
                DrawText(error_buffer, linelength / 2 - 100, height + 110, 20, DARKGRAY);
            }
        }

        EndDrawing();
    };

#if defined(PLATFORM_WEB)
    std::function<void()> u{update_func};
    emscripten_set_main_loop_arg(update_callback, &u, 0, 1);
#else
    while (!WindowShouldClose())
    {
        update_func();
    }
#endif

    CloseWindow();

    return 0;
}