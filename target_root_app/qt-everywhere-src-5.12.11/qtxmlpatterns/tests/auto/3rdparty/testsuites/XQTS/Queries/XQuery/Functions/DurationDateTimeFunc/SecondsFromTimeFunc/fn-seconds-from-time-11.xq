(:*******************************************************:)
(:Test: seconds-from-time-11                             :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 6, 2005                                     :)
(:Purpose: Evaluates The "seconds-from-time" function    :)
(:as part of a "div" expression.                         :) 
(:*******************************************************:)

fn:seconds-from-time(xs:time("22:33:10Z")) div fn:seconds-from-time(xs:time("02:11:02Z"))
