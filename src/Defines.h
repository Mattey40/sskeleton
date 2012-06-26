#include <algorithm>
#include <cfloat>
#include <cmath>
#include <cstdlib>

/* Floating point comparisons (combines test for absolute error and relative        
 * error)                                                                           
 */                                                                                 
inline bool fIsApprox(float a, float b)                                             
{                                                                                   
    return (fabsf(a - b) <= (FLT_EPSILON * std::max(std::max(1.0f, fabsf(a)),             
                    fabsf(b))));                                                            
}                                                                                   

inline bool dIsApprox(double a, double b)                                           
{                                                                                   
    return (fabs(a - b) <= (DBL_EPSILON * std::max(std::max(1.0, fabs(a)), fabs(b))));   
}  
