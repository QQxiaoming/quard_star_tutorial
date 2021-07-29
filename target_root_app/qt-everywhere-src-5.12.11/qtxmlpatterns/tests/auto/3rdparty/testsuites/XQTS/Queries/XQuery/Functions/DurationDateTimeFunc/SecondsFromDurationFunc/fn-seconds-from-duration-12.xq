(:*******************************************************:)
(:Test: fn-seconds-from-duration-12                      :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 13, 2005                                    :)
(:Purpose: Evaluates The "seconds-from-duration" function:)
(:as part of a "idiv" expression.                        :) 
(:*******************************************************:)

fn:seconds-from-duration(xs:dayTimeDuration("P25DT10H20M40S")) idiv fn:seconds-from-duration(xs:dayTimeDuration("P05DT02H04M20S"))
