(:*******************************************************:)
(:Test: fn-month-from-dateTime-10                        :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 8, 2005                                     :)
(:Purpose: Evaluates The "month-from-dateTime" function  :)
(:as part of a "*" expression.                           :) 
(:*******************************************************:)

fn:month-from-dateTime(xs:dateTime("1970-03-01T02:00:00Z")) * fn:month-from-dateTime(xs:dateTime("0002-02-01T10:00:00Z"))
