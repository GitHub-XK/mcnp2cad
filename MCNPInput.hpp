#ifndef MCNP_INPUT_FORMAT_H
#define MCNP_INPUT_FORMAT_H

#include <vector>
#include <map>
#include <iosfwd>
#include <string>

typedef std::vector< std::string > token_list_t;

class InputDeck;

template <class T> class DataRef{

public:
  virtual ~DataRef(){}

  virtual bool hasData() const { return true; }
  virtual const T& getData() const = 0;
  virtual DataRef<T>* clone() = 0;

};

/**
 * Superclass of all cards in the input deck
 */
class Card{
protected:
  InputDeck& parent_deck;

  Card( InputDeck& deck_p ):
    parent_deck(deck_p)
  {}

  virtual ~Card(){}

public:
  InputDeck& getDeck() { return parent_deck; }

};

// forward defs
class Transform;
class Lattice;

/**
 * Cell card
 */
class CellCard : public Card {

public:
  enum geom_token_t {INTERSECT, UNION, COMPLEMENT, LPAREN, RPAREN, CELLNUM, SURFNUM};
  typedef std::pair<enum geom_token_t, int> geom_list_entry_t;
  typedef std::vector<geom_list_entry_t> geom_list_t;
  
protected:
  
  CellCard( InputDeck& deck );

private: 
  // never defined and should never be called
  CellCard( const CellCard& c );
  CellCard& operator=( const CellCard& c );

public:
  
  virtual int getIdent() const = 0; 
  virtual const geom_list_t getGeom() const = 0; 

  virtual const DataRef<Transform>& getTrcl() const = 0; 
  virtual int getUniverse() const = 0;
  virtual bool hasFill() const = 0;
  virtual const Lattice& getFill() const = 0;

  virtual void print( std::ostream& s ) const = 0; 

};

std::ostream& operator<<(std::ostream& str, const CellCard::geom_list_entry_t& t );

class AbstractSurface;


/**
 * Surface Card
 */
class SurfaceCard : public Card {
  protected:
  int ident;//, coord_xform;
  DataRef<Transform> *coord_xform;
  std::string mnemonic;
  std::vector<double> args;
  AbstractSurface* surface;

public:
  SurfaceCard( InputDeck& deck, const token_list_t tokens );

  int getIdent() const { return ident; } 
					     
  void print( std::ostream& s ) const ;

  AbstractSurface& getSurface();
  const DataRef<Transform>& getTransform();

};

/**
 * Data cards
 */

class DataCard : public Card {

public:
  typedef enum { TR, OTHER } kind;
  typedef std::pair< kind, int > id_t;

  DataCard( InputDeck& deck ) : Card( deck ) {}

  virtual void print( std::ostream& str ) = 0;
  virtual kind getKind(){ return OTHER; }

};


/**
 * Main interface to MCNP reader: the InputDeck 
 */
class InputDeck{

public:
  typedef std::vector< CellCard* > cell_card_list;
  typedef std::vector< SurfaceCard* > surface_card_list;
  typedef std::vector< DataCard* > data_card_list;

protected:
  class LineExtractor;

  cell_card_list cells;
  surface_card_list surfaces;
  data_card_list datacards;

  std::map<int, CellCard*> cell_map;
  std::map<int, SurfaceCard*> surface_map;
  std::map< DataCard::id_t, DataCard*> datacard_map;

  void parseTitle( LineExtractor& lines );
  void parseCells( LineExtractor& lines );
  void parseSurfaces( LineExtractor& lines );
  void parseDataCards( LineExtractor& lines );

public:

  ~InputDeck();

  cell_card_list& getCells() { return cells; }
  surface_card_list& getSurfaces() { return surfaces; } 
  data_card_list& getDataCards(){ return datacards; }

  cell_card_list getCellsOfUniverse( int universe );

  CellCard* lookup_cell_card(int ident){
    return (*cell_map.find(ident)).second;
  }

  SurfaceCard* lookup_surface_card(int ident){
    return (*surface_map.find(ident)).second;
  }

  DataCard* lookup_data_card( const DataCard::id_t& ident ){
    return (*datacard_map.find(ident)).second;
  }

  DataCard* lookup_data_card( DataCard::kind k, int ident ){
    return lookup_data_card( std::make_pair( k, ident ) );
  }

  static InputDeck& build( std::istream& input );
  
  void createGeometry();


};

template < class T >
std::ostream& operator<<( std::ostream& out, const std::vector<T>& list );

#endif /* MCNP_INPUT_FORMAT_H */
