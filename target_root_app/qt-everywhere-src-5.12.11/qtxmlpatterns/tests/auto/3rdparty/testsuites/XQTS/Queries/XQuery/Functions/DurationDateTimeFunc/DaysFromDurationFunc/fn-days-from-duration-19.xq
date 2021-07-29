(:*******************************************************:)
(:Test: fn-days-from-duration-19                         :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 13, 2005                                    :)
(:Purpose: Evaluates The "days-from-duration" function   :)
(:as part of a "numeric-equal" expression (ge operator)  :) 
(:*******************************************************:)

fn:days-from-duration(xs:dayTimeDuration("P21DT07H")) ge fn:days-from-duration(xs:dayTimeDuration("P20DT01H"))