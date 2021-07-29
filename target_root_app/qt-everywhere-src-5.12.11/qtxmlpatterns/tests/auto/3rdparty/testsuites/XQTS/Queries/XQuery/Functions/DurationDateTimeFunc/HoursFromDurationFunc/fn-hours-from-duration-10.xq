(:*******************************************************:)
(:Test: fn-hours-from-duration-10                        :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 13, 2005                                    :)
(:Purpose: Evaluates The "hours-from-duration" function  :)
(:as part of a "*" expression.                           :) 
(:*******************************************************:)

fn:hours-from-duration(xs:dayTimeDuration("P20DT05H")) * fn:hours-from-duration(xs:dayTimeDuration("P03DT08H"))