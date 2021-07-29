(:*******************************************************:)
(:Test: fn-seconds-from-duration-19                      :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 13, 2005                                    :)
(:Purpose: Evaluates The "seconds-from-duration" function:)
(:as part of a "numeric-equal" expression (ge operator)  :) 
(:*******************************************************:)

fn:seconds-from-duration(xs:dayTimeDuration("P21DT07H12M59S")) ge fn:seconds-from-duration(xs:dayTimeDuration("P20DT01H13M01S"))