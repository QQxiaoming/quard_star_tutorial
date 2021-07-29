(:*******************************************************:)
(:Test: fn-month-from-dateTime-8                         :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 8, 2005                                     :)
(:Purpose: Evaluates The "month-from-dateTime" function  :)
(:as part of a "+" expression.                           :) 
(:*******************************************************:)

fn:month-from-dateTime(xs:dateTime("1970-01-01T00:00:00Z")) + fn:month-from-dateTime(xs:dateTime("1970-01-01T10:00:00Z"))
