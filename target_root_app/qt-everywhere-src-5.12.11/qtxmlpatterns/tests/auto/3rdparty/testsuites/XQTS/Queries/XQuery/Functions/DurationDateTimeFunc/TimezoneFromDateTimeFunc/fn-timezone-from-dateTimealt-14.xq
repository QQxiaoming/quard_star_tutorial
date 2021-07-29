(:*******************************************************:)
(:Test: fn-timezone-from-dateTime-14                     :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 27, 2005                                    :)
(:Purpose: Evaluates The "timezone-from-dateTime" function  :)
(:as an argument to the "fn:string" function             :) 
(:*******************************************************:)

fn:string(fn:timezone-from-dateTime(xs:dateTime("1970-01-01T10:00:00Z")))
