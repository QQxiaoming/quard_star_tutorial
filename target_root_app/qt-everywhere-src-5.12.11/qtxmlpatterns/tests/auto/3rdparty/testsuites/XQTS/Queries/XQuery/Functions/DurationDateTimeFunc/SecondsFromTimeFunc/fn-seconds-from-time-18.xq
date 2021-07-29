(:*******************************************************:)
(:Test: seconds-from-time-18                             :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 6, 2005                                     :)
(:Purpose: Evaluates The "seconds-from-time" function    :)
(:as part of a "numeric-equal" expression (le operator)  :) 
(:*******************************************************:)

fn:seconds-from-time(xs:time("10:00:00Z")) le fn:seconds-from-time(xs:time("10:00:00Z"))
