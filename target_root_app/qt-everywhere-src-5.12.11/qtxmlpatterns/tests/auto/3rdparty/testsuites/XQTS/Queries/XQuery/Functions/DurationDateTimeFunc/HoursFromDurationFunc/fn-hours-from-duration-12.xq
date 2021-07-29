(:*******************************************************:)
(:Test: fn-hours-from-duration-12                         :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 13, 2005                                    :)
(:Purpose: Evaluates The "hours-from-duration" function   :)
(:as part of a "idiv" expression.                        :) 
(:*******************************************************:)

fn:hours-from-duration(xs:dayTimeDuration("P25DT10H")) idiv fn:hours-from-duration(xs:dayTimeDuration("P05DT02H"))
