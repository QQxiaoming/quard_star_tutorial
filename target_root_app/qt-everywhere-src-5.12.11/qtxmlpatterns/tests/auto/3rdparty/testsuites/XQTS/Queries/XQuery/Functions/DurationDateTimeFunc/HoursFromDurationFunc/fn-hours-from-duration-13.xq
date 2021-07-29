(:*******************************************************:)
(:Test: fn-hours-from-duration-13                        :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 13, 2005                                    :)
(:Purpose: Evaluates The "hours-from-duration" function  :)
(:as part of a "mod" expression.                         :) 
(:*******************************************************:)

fn:hours-from-duration(xs:dayTimeDuration("P10DT10H")) mod fn:hours-from-duration(xs:dayTimeDuration("P03DT02H"))
