(:*******************************************************:)
(:Test: fn-hours-from-duration-18                        :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 13, 2005                                    :)
(:Purpose: Evaluates The "hours-from-duration" function  :)
(:as part of a "numeric-equal" expression (le operator)  :) 
(:*******************************************************:)

fn:hours-from-duration(xs:dayTimeDuration("P20DT03H")) le fn:hours-from-duration(xs:dayTimeDuration("P21DT01H"))