(:*******************************************************:)
(:Test: fn-minutes-from-duration-19                      :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 13, 2005                                    :)
(:Purpose: Evaluates The "minutes-from-duration" function:)
(:as part of a "numeric-equal" expression (ge operator)  :) 
(:*******************************************************:)

fn:minutes-from-duration(xs:dayTimeDuration("P21DT07H12M")) ge fn:minutes-from-duration(xs:dayTimeDuration("P20DT01H13M"))