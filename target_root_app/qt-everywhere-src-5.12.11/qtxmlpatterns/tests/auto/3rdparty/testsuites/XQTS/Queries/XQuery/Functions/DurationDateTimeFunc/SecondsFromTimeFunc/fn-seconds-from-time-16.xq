(:*******************************************************:)
(:Test: seconds-from-time-16                             :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 6, 2005                                     :)
(:Purpose: Evaluates The "seconds-from-time" function    :)
(:as part of a "numeric-equal" expression (eq operator)  :) 
(:*******************************************************:)

fn:seconds-from-time(xs:time("10:02:01Z")) eq fn:seconds-from-time(xs:time("10:02:00Z"))
