(:*******************************************************:)
(:Test: fn-hours-from-duration-17                        :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 13, 2005                                    :)
(:Purpose: Evaluates The "hours-from-duration" function  :)
(:as part of a "numeric-equal" expression (ne operator)  :) 
(:*******************************************************:)

fn:hours-from-duration(xs:dayTimeDuration("P23DT07H")) ne fn:hours-from-duration(xs:dayTimeDuration("P12DT05H"))
