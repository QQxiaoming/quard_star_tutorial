(:*******************************************************:)
(:Test: fn-hours-from-duration-19                        :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 13, 2005                                    :)
(:Purpose: Evaluates The "hours-from-duration" function  :)
(:as part of a "numeric-equal" expression (ge operator)  :) 
(:*******************************************************:)

fn:hours-from-duration(xs:dayTimeDuration("P21DT07H")) ge fn:hours-from-duration(xs:dayTimeDuration("P20DT08H"))