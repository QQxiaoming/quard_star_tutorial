(:*******************************************************:)
(:Test: fn-seconds-from-duration-10                      :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 13, 2005                                    :)
(:Purpose: Evaluates The "seconds-from-duration" function:)
(:as part of a "*" expression.                           :) 
(:*******************************************************:)

fn:seconds-from-duration(xs:dayTimeDuration("P20DT09H04M20S")) * fn:seconds-from-duration(xs:dayTimeDuration("P03DT10H10M03S"))