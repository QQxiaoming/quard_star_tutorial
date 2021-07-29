(:*******************************************************:)
(:Test: minutes-from-time-12                             :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 6, 2005                                     :)
(:Purpose: Evaluates The "minutes-from-time" function    :)
(:as part of a "idiv" expression.                        :) 
(:*******************************************************:)

fn:minutes-from-time(xs:time("10:12:00Z")) idiv fn:minutes-from-time(xs:time("02:02:00Z"))
