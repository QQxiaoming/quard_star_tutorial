(:*******************************************************:)
(:Test: fn-seconds-from-duration-11                      :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 13, 2005                                    :)
(:Purpose: Evaluates The "seconds-from-duration" function:)
(:as part of a "div" expression.                         :) 
(:*******************************************************:)

fn:seconds-from-duration(xs:dayTimeDuration("P20DT10H10M30S")) div fn:seconds-from-duration(xs:dayTimeDuration("P05DT05H02M02S"))
