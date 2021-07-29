(:*******************************************************:)
(:Test: fn-minutes-from-duration-17                      :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 13, 2005                                    :)
(:Purpose: Evaluates The "minutes-from-duration" function:)
(:as part of a "numeric-equal" expression (ne operator)  :) 
(:*******************************************************:)

fn:minutes-from-duration(xs:dayTimeDuration("P23DT08H20M")) ne fn:minutes-from-duration(xs:dayTimeDuration("P12DT05H22M"))
