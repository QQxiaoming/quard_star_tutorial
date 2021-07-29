(:*******************************************************:)
(:Test: fn-days-from-duration-17                         :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 13, 2005                                    :)
(:Purpose: Evaluates The "days-from-duration" function   :)
(:as part of a "numeric-equal" expression (ne operator)  :) 
(:*******************************************************:)

fn:days-from-duration(xs:dayTimeDuration("P23DT08H")) ne fn:days-from-duration(xs:dayTimeDuration("P12DT05H"))
