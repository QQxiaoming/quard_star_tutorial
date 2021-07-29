(:*******************************************************:)
(:Test: fn-hours-from-duration-16                        :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 13, 2005                                    :)
(:Purpose: Evaluates The "hours-from-duration" function  :)
(:as part of a "numeric-equal" expression (eq operator)  :) 
(:*******************************************************:)

fn:hours-from-duration(xs:dayTimeDuration("P22DT09H")) eq fn:hours-from-duration(xs:dayTimeDuration("P22DT09H"))
