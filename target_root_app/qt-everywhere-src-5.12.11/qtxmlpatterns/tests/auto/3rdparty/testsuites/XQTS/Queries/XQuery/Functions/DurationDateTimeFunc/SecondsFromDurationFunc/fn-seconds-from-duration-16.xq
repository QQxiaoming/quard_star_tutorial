(:*******************************************************:)
(:Test: fn-seconds-from-duration-16                      :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 13, 2005                                    :)
(:Purpose: Evaluates The "seconds-from-duration" function:)
(:as part of a "numeric-equal" expression (eq operator)  :) 
(:*******************************************************:)

fn:seconds-from-duration(xs:dayTimeDuration("P22DT10H10M01S")) eq fn:seconds-from-duration(xs:dayTimeDuration("P22DT09H10M01S"))
