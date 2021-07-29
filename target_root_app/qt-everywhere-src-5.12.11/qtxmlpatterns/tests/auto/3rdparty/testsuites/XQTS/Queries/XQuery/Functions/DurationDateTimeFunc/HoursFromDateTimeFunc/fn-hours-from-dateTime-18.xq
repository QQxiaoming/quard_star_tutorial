(:*******************************************************:)
(:Test: hours-from-dateTime-18                           :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 6, 2005                                     :)
(:Purpose: Evaluates The "hours-from-dateTime" function  :)
(:as part of a "numeric-equal" expression (le operator)  :) 
(:*******************************************************:)

fn:hours-from-dateTime(xs:dateTime("1970-01-01T10:00:00Z")) le fn:hours-from-dateTime(xs:dateTime("1970-01-01T10:00:00Z"))
