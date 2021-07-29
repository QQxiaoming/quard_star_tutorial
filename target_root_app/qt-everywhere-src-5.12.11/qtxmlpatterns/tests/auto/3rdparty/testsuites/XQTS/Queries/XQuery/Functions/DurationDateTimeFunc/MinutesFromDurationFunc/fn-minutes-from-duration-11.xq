(:*******************************************************:)
(:Test: fn-minutes-from-duration-11                      :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 13, 2005                                    :)
(:Purpose: Evaluates The "minutes-from-duration" function:)
(:as part of a "div" expression.                         :) 
(:*******************************************************:)

fn:minutes-from-duration(xs:dayTimeDuration("P20DT10H10M")) div fn:minutes-from-duration(xs:dayTimeDuration("P05DT05H02M"))
