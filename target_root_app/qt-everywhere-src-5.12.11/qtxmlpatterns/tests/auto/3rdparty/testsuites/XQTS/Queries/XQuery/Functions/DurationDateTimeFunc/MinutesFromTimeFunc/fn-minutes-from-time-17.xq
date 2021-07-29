(:*******************************************************:)
(:Test: minutes-from-time-17                             :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 6, 2005                                     :)
(:Purpose: Evaluates The "minutes-from-time" function    :)
(:as part of a "numeric-equal" expression (ne operator)  :) 
(:*******************************************************:)

fn:minutes-from-time(xs:time("10:00:00Z")) ne fn:minutes-from-time(xs:time("01:01:00Z"))
