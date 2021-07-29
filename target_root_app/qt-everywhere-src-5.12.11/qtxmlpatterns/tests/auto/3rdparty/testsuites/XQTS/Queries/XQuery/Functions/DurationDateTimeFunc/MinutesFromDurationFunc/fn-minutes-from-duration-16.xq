(:*******************************************************:)
(:Test: fn-minutes-from-duration-16                      :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 13, 2005                                    :)
(:Purpose: Evaluates The "minutes-from-duration" function:)
(:as part of a "numeric-equal" expression (eq operator)  :) 
(:*******************************************************:)

fn:minutes-from-duration(xs:dayTimeDuration("P22DT10H10M")) eq fn:minutes-from-duration(xs:dayTimeDuration("P22DT09H10M"))
