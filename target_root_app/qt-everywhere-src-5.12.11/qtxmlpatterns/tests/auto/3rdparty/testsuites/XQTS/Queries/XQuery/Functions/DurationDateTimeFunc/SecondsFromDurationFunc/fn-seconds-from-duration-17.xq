(:*******************************************************:)
(:Test: fn-seconds-from-duration-17                      :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 13, 2005                                    :)
(:Purpose: Evaluates The "seconds-from-duration" function:)
(:as part of a "numeric-equal" expression (ne operator)  :) 
(:*******************************************************:)

fn:seconds-from-duration(xs:dayTimeDuration("P23DT08H20M02S")) ne fn:seconds-from-duration(xs:dayTimeDuration("P12DT05H22M03S"))
