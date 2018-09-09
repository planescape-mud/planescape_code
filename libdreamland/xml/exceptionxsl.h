/* $Id$
 *
 * ruffina, 2004
 */
/***************************************************************************
                          exceptionxsl.h  -  description
                             -------------------
    begin                : Mon Oct 15 2001
    copyright            : (C) 2001 by Igor S. Petrenko
    email                : nofate@europe.com
 ***************************************************************************/

#ifndef EXCEPTIONXSL_H
#define EXCEPTIONXSL_H

#include "exception.h"

/**
 * @short ��� ��������� ������ ��� ������� patterns
 * @author Igor S. Petrenko
 */
class ExceptionXSL : public Exception
{
public: 
    /**
     * @arg type - ��������� � ������
     * @arg position - � ����� �������
     */
    ExceptionXSL( const string type, int position  );
    
    /**
     * @arg type - ��������� � ������
     * @arg symbol - ����������� ������
     * @arg position - � ����� �������
     */
    ExceptionXSL( const string type, const string  symbol, int position  );
    
    /**
     * @arg symbol - ����������� ������
     * @arg position - � ����� �������
     */
    ExceptionXSL( char symbol, int position  );

    virtual ~ExceptionXSL( ) throw( );
};

#endif
