(:*******************************************************:)
(:Test: fn-timezone-from-dateTime-10                     :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 27, 2005                                    :)
(:Purpose: Evaluates The "timezone-from-dateTime" function  :)
(:as an argument to the "min" function.                  :) 
(:*******************************************************:)

fn:min(fn:timezone-from-dateTime(xs:dateTime("1970-01-01T02:00:00Z")))
