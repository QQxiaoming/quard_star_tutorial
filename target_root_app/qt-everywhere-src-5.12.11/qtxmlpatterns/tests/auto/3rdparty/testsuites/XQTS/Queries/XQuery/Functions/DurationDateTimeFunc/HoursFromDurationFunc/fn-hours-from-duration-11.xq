(:*******************************************************:)
(:Test: fn-hours-from-duration-11                        :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 13, 2005                                    :)
(:Purpose: Evaluates The "hours-from-duration" function  :)
(:as part of a "div" expression.                         :) 
(:*******************************************************:)

fn:hours-from-duration(xs:dayTimeDuration("P20DT10H")) div fn:hours-from-duration(xs:dayTimeDuration("P05DT05H"))
