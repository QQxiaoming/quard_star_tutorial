(:*******************************************************:)
(:Test: fn-month-from-dateTime-13                        :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 8, 2005                                     :)
(:Purpose: Evaluates The "month-from-dateTime" function  :)
(:as part of a "mod" expression.                         :) 
(:*******************************************************:)

fn:month-from-dateTime(xs:dateTime("1970-10-01T10:00:00Z")) mod fn:month-from-dateTime(xs:dateTime("1970-03-01T03:00:00Z"))
