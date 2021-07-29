(:*******************************************************:)
(:Test: fn-timezone-from-time-12                         :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 6, 2005                                     :)
(:Purpose: Evaluates The "timezone-from-time" function as:)
(:an argument to the "fn:number" function.  Return NaN.  :)
(:*******************************************************:)

fn:number(fn:timezone-from-time(xs:time("10:12:15Z")))