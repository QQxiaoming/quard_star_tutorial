(:*******************************************************:)
(:Test: seconds-from-time-12                             :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 6, 2005                                     :)
(:Purpose: Evaluates The "seconds-from-time" function    :)
(:as part of a "idiv" expression.                        :) 
(:*******************************************************:)

fn:seconds-from-time(xs:time("10:12:15Z")) idiv fn:seconds-from-time(xs:time("02:02:03Z"))
