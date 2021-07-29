(:*******************************************************:)
(:Test: fn-minutes-from-duration-10                      :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 13, 2005                                    :)
(:Purpose: Evaluates The "minutes-from-duration" function:)
(:as part of a "*" expression.                           :) 
(:*******************************************************:)

fn:minutes-from-duration(xs:dayTimeDuration("P20DT09H04M")) * fn:minutes-from-duration(xs:dayTimeDuration("P03DT10H10M"))