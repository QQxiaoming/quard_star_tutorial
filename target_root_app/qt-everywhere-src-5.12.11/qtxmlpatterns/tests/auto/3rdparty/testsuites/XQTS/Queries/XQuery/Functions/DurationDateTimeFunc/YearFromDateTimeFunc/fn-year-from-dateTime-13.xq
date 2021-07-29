(:*******************************************************:)
(:Test: year-from-dateTime-13                            :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 8, 2005                                     :)
(:Purpose: Evaluates The "year-from-dateTime" function   :)
(:as part of a "mod" expression.                         :) 
(:*******************************************************:)

fn:year-from-dateTime(xs:dateTime("1970-01-01T10:00:00Z")) mod fn:year-from-dateTime(xs:dateTime("1970-01-01T03:00:00Z"))
