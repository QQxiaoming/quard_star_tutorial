(:*******************************************************:)
(:Test: fn-minutes-from-duration-13                      :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 13, 2005                                    :)
(:Purpose: Evaluates The "minutes-from-duration" function:)
(:as part of a "mod" expression.                         :) 
(:*******************************************************:)

fn:minutes-from-duration(xs:dayTimeDuration("P10DT10H20M")) mod fn:minutes-from-duration(xs:dayTimeDuration("P03DT03H03M"))
