(:*******************************************************:)
(:Test: fn-month-from-dateTime-18                        :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 8, 2005                                     :)
(:Purpose: Evaluates The "month-from-dateTime" function  :)
(:as part of a "numeric-equal" expression (le operator)  :) 
(:*******************************************************:)

fn:month-from-dateTime(xs:dateTime("1970-02-01T10:00:00Z")) le fn:month-from-dateTime(xs:dateTime("1971-01-01T10:00:00Z"))
