(:*******************************************************:)
(:Test: fn-timezone-from-dateTime-11                     :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 27, 2005                                     :)
(:Purpose: Evaluates The "timezone-from-dateTime" function  :)
(:as part of a "div" expression.                         :) 
(:*******************************************************:)

fn:timezone-from-dateTime(xs:dateTime("1970-01-01T02:00:00+10:00")) div fn:timezone-from-dateTime(xs:dateTime("1970-01-01T10:00:00+05:00"))
