(:*******************************************************:)
(:Test: fn-days-from-duration-18                         :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 13, 2005                                    :)
(:Purpose: Evaluates The "days-from-duration" function   :)
(:as part of a "numeric-equal" expression (le operator)  :) 
(:*******************************************************:)

fn:days-from-duration(xs:dayTimeDuration("P20DT03H")) le fn:days-from-duration(xs:dayTimeDuration("P21DT15H"))