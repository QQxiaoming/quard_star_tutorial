(:*******************************************************:)
(:Test: year-from-dateTime-12                            :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 8, 2005                                     :)
(:Purpose: Evaluates The "year-from-dateTime" function   :)
(:as part of a "idiv" expression.                        :) 
(:*******************************************************:)

fn:year-from-dateTime(xs:dateTime("1970-01-01T10:00:00Z")) idiv fn:year-from-dateTime(xs:dateTime("1970-01-01T02:00:00Z"))
