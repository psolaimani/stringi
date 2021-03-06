% Generated by roxygen2: do not edit by hand
% Please edit documentation in R/join.R
\name{\%s+\%}
\alias{\%s+\%}
\alias{\%stri+\%}
\title{Concatenate Two Character Vectors}
\usage{
e1 \%s+\% e2

e1 \%stri+\% e2
}
\arguments{
\item{e1}{a character vector or an object coercible to a character vector}

\item{e2}{a character vector or an object coercible to a character vector}
}
\value{
Returns a character vector.
}
\description{
Binary operators for joining (concatenating) two character vectors,
with a typical \R look-and-feel.
}
\details{
Vectorized over \code{e1} and \code{e2}.

These operators act like a call to \code{\link{stri_join}(e1, e2, sep="")}.
However, note that joining 3 vectors, e.g. \code{e1 \%+\% e2 \%+\% e3}
is slower than \code{\link{stri_join}(e1, e2, e3, sep="")},
because it creates a new (temporary) result vector each time
the operator is applied.
}
\examples{
c('abc', '123', '\\u0105\\u0104') \%stri+\% letters[1:6]
'ID_' \%stri+\% 1:5

}
