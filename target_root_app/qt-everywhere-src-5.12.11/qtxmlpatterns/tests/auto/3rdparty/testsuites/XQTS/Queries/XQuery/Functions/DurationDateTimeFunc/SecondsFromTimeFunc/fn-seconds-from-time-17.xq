(:*******************************************************:)
(:Test: seconds-from-time-17                             :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 6, 2005                                     :)
(:Purpose: Evaluates The "seconds-from-time" function    :)
(:as part of a "numeric-equal" expression (ne operator)  :) 
(:*******************************************************:)

fn:seconds-from-time(xs:time("10:00:01Z")) ne fn:seconds-from-time(xs:time("01:01:00Z"))
