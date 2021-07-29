(:*******************************************************:)
(:Test: fn-month-from-dateTime-19                        :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 8, 2005                                     :)
(:Purpose: Evaluates The "month-from-dateTime" function  :)
(:as part of a "numeric-equal" expression (ge operator)  :) 
(:*******************************************************:)

fn:month-from-dateTime(xs:dateTime("1971-01-01T10:00:00Z")) ge fn:month-from-dateTime(xs:dateTime("1970-01-01T10:00:00Z"))
