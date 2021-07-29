(:*******************************************************:)
(:Test: minutes-from-time-16                             :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 6, 2005                                     :)
(:Purpose: Evaluates The "minutes-from-time" function    :)
(:as part of a "numeric-equal" expression (eq operator)  :) 
(:*******************************************************:)

fn:minutes-from-time(xs:time("10:02:00Z")) eq fn:minutes-from-time(xs:time("10:02:00Z"))
