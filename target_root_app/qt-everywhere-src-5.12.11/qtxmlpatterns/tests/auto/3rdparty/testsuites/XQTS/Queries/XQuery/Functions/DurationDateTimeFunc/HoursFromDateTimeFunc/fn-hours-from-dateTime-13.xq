(:*******************************************************:)
(:Test: hours-from-dateTime-13                           :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 6, 2005                                     :)
(:Purpose: Evaluates The "hours-from-dateTime" function  :)
(:as part of a "mod" expression.                         :) 
(:*******************************************************:)

fn:hours-from-dateTime(xs:dateTime("1970-01-01T10:00:00Z")) mod fn:hours-from-dateTime(xs:dateTime("1970-01-01T03:00:00Z"))
