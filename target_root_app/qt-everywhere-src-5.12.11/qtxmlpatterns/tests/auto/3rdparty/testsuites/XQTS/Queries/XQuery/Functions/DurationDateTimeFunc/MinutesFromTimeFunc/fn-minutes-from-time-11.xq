(:*******************************************************:)
(:Test: minutes-from-time-11                             :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 6, 2005                                     :)
(:Purpose: Evaluates The "minutes-from-time" function    :)
(:as part of a "div" expression.                         :) 
(:*******************************************************:)

fn:minutes-from-time(xs:time("22:33:00Z")) div fn:minutes-from-time(xs:time("02:11:00Z"))
