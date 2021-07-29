(:*******************************************************:)
(:Test: fn-month-from-dateTime-11                        :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 8, 2005                                     :)
(:Purpose: Evaluates The "month-from-dateTime" function  :)
(:as part of a "div" expression.                         :) 
(:*******************************************************:)

fn:month-from-dateTime(xs:dateTime("1000-10-01T02:00:00Z")) div fn:month-from-dateTime(xs:dateTime("0050-05-01T10:00:00Z"))
