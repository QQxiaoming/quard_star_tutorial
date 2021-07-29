(:*******************************************************:)
(:Test: fn-month-from-dateTime-12                        :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 8, 2005                                     :)
(:Purpose: Evaluates The "month-from-dateTime" function  :)
(:as part of a "idiv" expression.                        :) 
(:*******************************************************:)

fn:month-from-dateTime(xs:dateTime("1970-01-12T10:00:00Z")) idiv fn:month-from-dateTime(xs:dateTime("1970-02-01T02:00:00Z"))
