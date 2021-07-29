(:*******************************************************:)
(:Test: hours-from-dateTime-11                           :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 6, 2005                                     :)
(:Purpose: Evaluates The "hours-from-dateTime" function  :)
(:as part of a "div" expression.                         :) 
(:*******************************************************:)

fn:hours-from-dateTime(xs:dateTime("1970-01-01T02:00:00Z")) div fn:hours-from-dateTime(xs:dateTime("1970-01-01T10:00:00Z"))
