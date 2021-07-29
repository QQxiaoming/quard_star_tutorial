(:*******************************************************:)
(:Test: fn-timezone-from-dateTime-13                     :)
(:Date: June 27, 2005                                    :)
(:Purpose: Evaluates The "timezone-from-dateTime" function  :)
(:as as an argument to an "fn:max" function.             :) 
(:*******************************************************:)

fn:max(fn:timezone-from-dateTime(xs:dateTime("1970-01-01T10:00:00Z")))

